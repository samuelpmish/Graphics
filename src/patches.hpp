#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "rgbcolor.hpp"
#include "vertex.hpp"

namespace Graphics {

enum PatchType { TRI6, QUAD4, QUAD8, QUAD9 };

using Tri6 = std::array< glm::vec3, 6 >;
using Quad4 = std::array< glm::vec3, 4 >;
using Quad8 = std::array< glm::vec3, 8 >;
using Quad9 = std::array< glm::vec3, 9 >;

using Tri6v = std::array< glm::vec4, 6 >;
using Quad4v = std::array< glm::vec4, 4 >;
using Quad8v = std::array< glm::vec4, 8 >;
using Quad9v = std::array< glm::vec4, 9 >;

struct Patches {

  Patches();
  void draw(const Camera & camera);

  void clear();

  void append(const Tri6 & patch);
  void append(const Quad4 & patch);
  void append(const Quad8 & patch);
  void append(const Quad9 & patch);

  void append(const Tri6v & patch);
  void append(const Quad4v & patch);
  void append(const Quad8v & patch);
  void append(const Quad9v & patch);

  //template < size_t n >
  //void append(const std::array< glm::vec3, n > & patch, const std::array< rgbcolor, n > & colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  void posterization(int i) { posterize = i; }

  void set_palette(std::vector< rgbcolor > p) { 
    palette = p;
  }

  void set_value_bounds(float min, float max) { 
    interval[0] = min;
    interval[1] = max;
  }
  
  void set_subdivision(PatchType p, int s) { 
    int clamped = std::max(1, std::min(s, 16)); 
    groups[0][p].subdivision = clamped;
    groups[1][p].subdivision = clamped;
  }

 private:

  struct RenderGroup {
    RenderGroup(const std::vector<std::string> & shaders, bool palette = false);

    ShaderProgram program;

    bool dirty;
    GLuint vao;
    GLuint color_vbo;
    GLuint position_vbo;
    GLuint texture;

    float subdivision;
    std::vector< float > values;
    std::vector< rgbcolor > colors;
    std::vector< glm::vec3 > positions;
  };

  rgbcolor color;
  glm::vec4 light;

  int posterize;
  float interval[2];
  std::vector< rgbcolor > palette;

  RenderGroup groups[2][4];

};

}