#pragma once

#include "clock_config.h"
#include <cairomm/context.h>
#include <gtkmm/drawingarea.h>

class Clock : public Gtk::DrawingArea {
public:
  explicit Clock(const ClockConfig &config = ClockConfig());
  virtual ~Clock() = default;

  void set_font_family(const std::string &family);
  void set_font_size(double size);
  void set_text_color(double r, double g, double b, double a);

  const std::string &font_family() const { return m_config.font_family; }
  double font_size() const { return m_config.font_size; }

  inline int get_width() const { return s_cached_width; }
  inline int get_height() const { return s_cached_height; }

protected:
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
  bool on_timeout();

private:
  void calculate_window_size(int &width, int &height);
  void calculate_next_update();
  void invalidate_cache();

  // Geometry of the time line alone, i.e. the window size the upstream
  // single-line clock would have used. The time's font size is always derived
  // from these, so adding the date line never resizes the time.
  static int s_base_width;
  static int s_base_height;

  // Picks the date font size that makes the date's rendered width match the
  // time's rendered width, never exceeding base_size.
  double compute_date_font_size(const Cairo::RefPtr<Cairo::Context> &cr,
                                double base_size, const std::string &time_str,
                                const std::string &date_str) const;

  ClockConfig m_config;
  time_t m_next_update;

  // Cache variables
  static Cairo::TextExtents s_text_extents;
  static bool s_size_calculated;
  static int s_cached_width;
  static int s_cached_height;
  static double s_cached_font_size;
};
