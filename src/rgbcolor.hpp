#pragma once

#include <array>
#include <inttypes.h>

#include <iostream>

struct rgbcolor {
  constexpr rgbcolor() : r(255), g(255), b(255), a(255) {}
  constexpr rgbcolor(uint8_t rgba[4]) : r{rgba[0]}, g{rgba[0]}, b{rgba[0]}, a{rgba[0]} {} 
  constexpr rgbcolor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) : r{r_}, g{g_}, b{b_}, a{a_} {} 
  rgbcolor(const char (& hex)[9]) {
    std::cout << std::string(hex, hex+1) << std::endl;
    r = std::stoi(std::string(hex+0, hex+2), nullptr, 16);
    g = std::stoi(std::string(hex+2, hex+4), nullptr, 16);
    b = std::stoi(std::string(hex+4, hex+6), nullptr, 16);
    a = std::stoi(std::string(hex+6, hex+8), nullptr, 16);
  }
  uint8_t r, g, b, a;
};

using color3 = std::array<rgbcolor, 3>;

namespace colors {
  constexpr rgbcolor gray(uint8_t value) { return rgbcolor{value, value, value, 255}; }

  inline constexpr rgbcolor white{255, 255, 255, 255};
  inline constexpr rgbcolor off_white{230, 230, 230, 255};
  inline constexpr rgbcolor light_gray{170, 170, 170, 255};

  inline constexpr rgbcolor red{230,  30,  30, 255};
  inline constexpr rgbcolor orange{240, 140,   0, 255};
  inline constexpr rgbcolor yellow{240, 220,   0, 255};
  inline constexpr rgbcolor green{30,  230,  30, 255};
  inline constexpr rgbcolor blue{ 30,  30, 200, 255};
  inline constexpr rgbcolor purple{170,  20, 170, 255};

};

namespace palettes {
  const std::vector< rgbcolor > warm = {
    rgbcolor("FFF7ECFF"), rgbcolor("FEE8C8FF"), rgbcolor("FDD49EFF"), rgbcolor("FDBB84FF"), rgbcolor("FC8D59FF"), rgbcolor("EF6548FF"), rgbcolor("D7301FFF"), rgbcolor("990000FF")
  };

  const std::vector< rgbcolor > cold {
    rgbcolor("F7FCF0FF"), rgbcolor("E0F3DBFF"), rgbcolor("CCEBC5FF"), rgbcolor("A8DDB5FF"), rgbcolor("7BCCC4FF"), rgbcolor("4EB3D3FF"), rgbcolor("2B8CBEFF"), rgbcolor("08589EFF") 
  };

  const std::vector< rgbcolor > blue_to_red {
    rgbcolor("0570B0FF"), rgbcolor("3690C0FF"), rgbcolor("74A9CFFF"), rgbcolor("A6BDDBFF"), rgbcolor("D0D1E6FF"), rgbcolor("ECE7F2FF"), rgbcolor("FFF7ECFF"), rgbcolor("FEE8C8FF"), rgbcolor("FDD49EFF"), rgbcolor("FDBB84FF"), rgbcolor("FC8D59FF"), rgbcolor("EF6548FF"), rgbcolor("D7301FFF")
  };

  const std::vector< rgbcolor > purple_to_yellow {
    rgbcolor("980B63FF"), rgbcolor("CA5FA7FF"), rgbcolor("F596FDFF"), rgbcolor("F7D8FDFF"), rgbcolor("FAD932FF")
  };

}