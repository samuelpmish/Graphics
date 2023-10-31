#pragma once

#include <array>
#include <inttypes.h>

struct rgbcolor {
  uint8_t r, g, b, a;
};

using color3 = std::array<rgbcolor, 3>;

namespace colors {
  constexpr rgbcolor gray(uint8_t value) { return rgbcolor{value, value, value, 255}; }

  inline constexpr rgbcolor white{255, 255, 255, 255};
  inline constexpr rgbcolor off_white{230, 230, 230, 255};
  inline constexpr rgbcolor light_gray{170, 170, 170, 255};
  inline constexpr rgbcolor blue{ 30,  30, 200, 255};
  inline constexpr rgbcolor red{230,  30,  30, 255};
  inline constexpr rgbcolor green{30,  230,  30, 255};
  inline constexpr rgbcolor orange{240, 140,   0, 255};
  inline constexpr rgbcolor yellow{240, 220,   0, 255};
  inline constexpr rgbcolor purple{170,  20, 170, 255};
};