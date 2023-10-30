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

struct Patches {

  Patches();
  void draw(const Camera & camera);

  void clear();

  void append(const Tri6 & patch);
  void append(const Quad4 & patch);
  void append(const Quad8 & patch);
  void append(const Quad9 & patch);

  //template < size_t n >
  //void append(const std::array< glm::vec3, n > & patch, const std::array< rgbcolor, n > & colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  auto size(PatchType t) { return groups[t].positions.size(); }
  
  void set_subdivision(PatchType p, int s) { groups[p].subdivision = std::max(1, std::min(s, 8)); }

 private:

  rgbcolor color;
  glm::vec4 light;

  struct RenderGroup {
    ShaderProgram program;

    bool dirty;
    GLuint vao;
    GLuint color_vbo;
    GLuint position_vbo;

    int subdivision;
    std::vector< rgbcolor > colors;
    std::vector< glm::vec3 > positions;
  };

  RenderGroup groups[4];

};

}