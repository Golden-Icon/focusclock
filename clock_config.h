#pragma once

#include <string>

struct ClockConfig {
  // Font settings
  std::string font_family = "Sans";
  double font_size = 32.0;
  double text_padding_ratio = 0.5;

  // Date line (rendered under the time, scaled to match the time's width)
  bool show_date = true;
  std::string date_format = "%x";
  // Fraction of the time font size below which the date is never scaled,
  // so a long date format cannot become illegible.
  double date_min_font_ratio = 0.35;
  // Vertical gap between the time and the date, as a fraction of font size.
  double line_gap_ratio = 0.15;

  // Colors
  double text_color[4] = {1.0, 1.0, 1.0, 0.5};
};
