#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "RGBColor.hpp"

namespace Graphics {

struct Triangle {
  glm::vec3 vertices[3];
};

struct TriangleWithColor {
  struct {
    glm::vec3 p;
    rgbcolor color;
  } vertices[3];
};

struct Triangles {

  Triangles();
  void draw(const Camera & camera);

  void clear();
  void append(const Triangle & triangle);
  void append(const std::vector< Triangle > & more_triangles);
  void append(const std::vector< Triangle > & more_triangles, const std::vector < rgbcolor > & more_colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  auto size() { return data.size(); }

 private:
  bool dirty;
  GLuint vao;
  GLuint normal_vbo;
  GLuint triangle_vbo;

  ShaderProgram program;

  rgbcolor color;

  std::vector< Triangle > normals;
  std::vector< TriangleWithColor > data;

  glm::vec4 light;

};

}