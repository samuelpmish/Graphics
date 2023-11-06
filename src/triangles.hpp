#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "rgbcolor.hpp"
#include "vertex.hpp"

namespace Graphics {

using Tri3 = std::array< glm::vec3, 3 >;


struct Triangles {

  Triangles();
  void draw(const Camera & camera);

  void clear();

  void append(const Tri3 & triangle);
  void append(const Tri3 & triangle, const std::array< rgbcolor, 3 > & colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  auto size() { return vertices.size(); }

 private:
  bool dirty;
  GLuint vao;
  GLuint normal_vbo;
  GLuint triangle_vbo;
  GLuint color_vbo;

  ShaderProgram program;

  rgbcolor color;

  std::vector< Tri3 > normals;
  std::vector< Tri3 > vertices;
  std::vector< color3 > colors;

  glm::vec4 light;

};

}