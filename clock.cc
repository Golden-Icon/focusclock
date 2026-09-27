#include "clock.h"
#include <algorithm>
#include <ctime>
#include <glibmm/main.h>
#include <glib.h>

Cairo::TextExtents Clock::s_text_extents;
bool Clock::s_size_calculated = false;
int Clock::s_cached_width = 0;
int Clock::s_cached_height = 0;
int Clock::s_base_width = 0;
int Clock::s_base_height = 0;
double Clock::s_cached_font_size = 0;

Clock::Clock(const ClockConfig &config) : m_config(config) {
  set_draw_func(sigc::mem_fun(*this, &Clock::on_draw));

  if (!s_size_calculated) {
    calculate_window_size(s_cached_width, s_cached_height);
    s_size_calculated = true;
  }
  set_size_request(s_cached_width, s_cached_height);

  calculate_next_update();
  Glib::signal_timeout().connect(sigc::mem_fun(*this, &Clock::on_timeout),
                                 1000);
}

void Clock::calculate_next_update() {
  time(&m_next_update);
  m_next_update += 60 - (m_next_update % 60); // Next minute
}

void Clock::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                    int height) {
  cr->set_operator(Cairo::Context::Operator::CLEAR);
  cr->paint();
  cr->set_operator(Cairo::Context::Operator::OVER);

  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime(&rawtime);

  static char buffer[6];
  strftime(buffer, sizeof(buffer), "%I:%M", timeinfo);

  char date_buffer[64] = {0};
  if (m_config.show_date) {
    strftime(date_buffer, sizeof(date_buffer), m_config.date_format.c_str(),
             timeinfo);
  }

  cr->select_font_face(m_config.font_family, Cairo::ToyFontFace::Slant::NORMAL,
                       Cairo::ToyFontFace::Weight::BOLD);

  double gap = m_config.font_size * m_config.line_gap_ratio;

  if (s_cached_font_size == 0) {
    // The fit calculation uses the geometry of the time line alone, never the
    // full window, so that the taller two-line window cannot shrink the time.
    int base_width = s_base_width > 0 ? s_base_width : width;
    int base_height = s_base_height > 0 ? s_base_height : height;

    // Calculate the ideal font size based on window dimensions
    double font_size = std::min(base_width * 0.4, base_height * 0.6);
    cr->set_font_size(font_size);
    cr->get_text_extents(buffer, s_text_extents);

    // Adjust font size if text is too large
    double scale_w = (base_width * 0.8) / s_text_extents.width;
    double scale_h = (base_height * 0.8) / s_text_extents.height;
    double scale = std::min(scale_w, scale_h);

    if (scale < 1.0) {
      font_size *= scale;
    }

    s_cached_font_size = font_size;
  }

  // The time and the date are laid out as one centred block so that they stay
  // visually anchored in the middle of the window as a single unit.
  Cairo::TextExtents time_extents;
  cr->set_font_size(s_cached_font_size);
  cr->get_text_extents(buffer, time_extents);

  Cairo::TextExtents date_extents;
  double date_font_size = 0;
  if (m_config.show_date && date_buffer[0] != '\0') {
    date_font_size =
        compute_date_font_size(cr, s_cached_font_size, buffer, date_buffer);
    cr->set_font_size(date_font_size);
    cr->get_text_extents(date_buffer, date_extents);
  }

  double block_height = time_extents.height;
  if (m_config.show_date && date_buffer[0] != '\0') {
    block_height += gap + date_extents.height;
  }

  double block_top = (height - block_height) / 2.0;

  cr->set_source_rgba(m_config.text_color[0], m_config.text_color[1],
                      m_config.text_color[2], m_config.text_color[3]);

  // Centre each line horizontally on its own rendered width, then place its
  // baseline so the ink starts exactly at the top of the block.
  // The font size must be re-applied here: measuring the date above left the
  // context set to date_font_size, which would otherwise draw the time at the
  // date's size.
  double x = (width - time_extents.width) / 2.0 - time_extents.x_bearing;
  double y = block_top - time_extents.y_bearing;
  cr->set_font_size(s_cached_font_size);
  cr->move_to(x, y);
  cr->show_text(buffer);

  if (m_config.show_date && date_buffer[0] != '\0') {
    double date_top = block_top + time_extents.height + gap;
    x = (width - date_extents.width) / 2.0 - date_extents.x_bearing;
    y = date_top - date_extents.y_bearing;
    cr->set_font_size(date_font_size);
    cr->move_to(x, y);
    cr->show_text(date_buffer);
  }

  if (g_getenv("FOCUSCLOCK_DEBUG_LAYOUT")) {
    // Report the size each line was actually drawn at, not the intended one.
    cr->set_font_size(s_cached_font_size);
    cairo_font_extents_t actual_time;
    cr->get_font_extents(actual_time);
    cr->set_font_size(date_font_size);
    cairo_font_extents_t actual_date;
    cr->get_font_extents(actual_date);
    g_printerr(
        "[layout] win=%dx%d time='%s' date='%s'\n"
        "[layout] time_font=%.3f (ascent %.2f) date_font=%.3f (ascent %.2f)\n"
        "[layout] time w=%.3f h=%.3f | date w=%.3f h=%.3f\n"
        "[layout] block=%.3f top=%.3f bottom=%.3f\n",
        width, height, buffer, date_buffer, s_cached_font_size,
        actual_time.ascent, date_font_size, actual_date.ascent,
        time_extents.width, time_extents.height, date_extents.width,
        date_extents.height, block_height, block_top, block_top + block_height);
  }
}

double Clock::compute_date_font_size(const Cairo::RefPtr<Cairo::Context> &cr,
                                     double base_size,
                                     const std::string &time_str,
                                     const std::string &date_str) const {
  if (date_str.empty()) {
    return base_size;
  }

  Cairo::TextExtents time_extents;
  Cairo::TextExtents date_extents;
  cr->set_font_size(base_size);
  cr->get_text_extents(time_str, time_extents);
  cr->get_text_extents(date_str, date_extents);

  if (date_extents.width <= 0 || time_extents.width <= 0) {
    return base_size;
  }

  // Width scales linearly with font size, so the ratio that matches the date's
  // width to the time's width is the ratio of the two measured widths.
  double scale = time_extents.width / date_extents.width;

  double min_size = base_size * m_config.date_min_font_ratio;
  return std::clamp(base_size * scale, min_size, base_size);
}

void Clock::calculate_window_size(int &width, int &height) {
  auto surface =
      Cairo::ImageSurface::create(Cairo::ImageSurface::Format::ARGB32, 1, 1);
  auto cr = Cairo::Context::create(surface);

  cr->select_font_face(m_config.font_family, Cairo::ToyFontFace::Slant::NORMAL,
                       Cairo::ToyFontFace::Weight::BOLD);
  cr->set_font_size(m_config.font_size);

  Cairo::TextExtents extents;
  cr->get_text_extents("00:00", extents);

  // Calculate base dimensions with padding
  double padding = m_config.font_size * m_config.text_padding_ratio;
  width = extents.width + padding * 2;
  height = extents.height + padding * 2;

  // Ensure minimum size
  width = std::max(width, 120);
  height = std::max(height, 60);

  // Freeze the time line's own geometry before the date line is accounted for.
  // The time's font size is derived from this, so it stays exactly the size the
  // single-line clock would have drawn it at.
  s_base_width = width;
  s_base_height = height;

  if (m_config.show_date) {
    // Measure the date at the size it will actually be drawn, using a sample
    // formatted from the current date. The date is scaled to the time's width,
    // so it never widens the window beyond what the time already requires.
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    char date_buffer[64] = {0};
    if (strftime(date_buffer, sizeof(date_buffer),
                 m_config.date_format.c_str(), timeinfo) > 0) {
      double date_font_size =
          compute_date_font_size(cr, m_config.font_size, "00:00", date_buffer);

      Cairo::TextExtents date_extents;
      cr->set_font_size(date_font_size);
      cr->get_text_extents(date_buffer, date_extents);

      double gap = m_config.font_size * m_config.line_gap_ratio;
      height += gap + date_extents.height;
    }
  }
}

bool Clock::on_timeout() {
  time_t now;
  time(&now);

  if (now >= m_next_update) {
    calculate_next_update();
    queue_draw();
  }

  return true;
}

void Clock::set_font_family(const std::string &family) {
  if (m_config.font_family != family) {
    m_config.font_family = family;
    invalidate_cache();
  }
}

void Clock::set_font_size(double size) {
  if (m_config.font_size != size) {
    m_config.font_size = size;
    invalidate_cache();
  }
}

void Clock::set_text_color(double r, double g, double b, double a) {
  if (m_config.text_color[0] != r || m_config.text_color[1] != g ||
      m_config.text_color[2] != b || m_config.text_color[3] != a) {
    m_config.text_color[0] = r;
    m_config.text_color[1] = g;
    m_config.text_color[2] = b;
    m_config.text_color[3] = a;
    queue_draw();
  }
}

void Clock::invalidate_cache() {
  s_size_calculated = false;
  s_cached_font_size = 0;
  queue_draw();
}
