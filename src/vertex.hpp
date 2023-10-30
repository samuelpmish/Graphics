#pragma once

#include <array>

#include "rgbcolor.hpp"

namespace Graphics {

  union color_value {
    rgbcolor color;
    float value;
  };

  struct vertex : public glm::vec3 {
    color_value cv;
  };

} // namespace Graphics