#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "rgbcolor.hpp"

#include "spheres.hpp"

namespace Graphics {

struct Cylinder {
  Sphere endpoints[2];
};

struct Cylinders {

  Cylinders();
  void draw(const Camera & camera);

  void clear();
  void append(const Cylinder & cylinder);
  void append(const std::vector< Cylinder > & more_cylinders);
  void append(const std::vector< Cylinder > & more_spheres, const std::vector < rgbcolor > & more_colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  auto size() { return data.size(); }

 private:
  bool dirty;
  GLuint vao;
  GLuint color_vbo;
  GLuint instance_vbo; 
  GLuint cylinder_vbo;

  ShaderProgram program;

  rgbcolor color;

  std::vector< Cylinder > data;
  std::vector< rgbcolor > colors;

  glm::vec4 light;

};

}