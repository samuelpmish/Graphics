#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "rgbcolor.hpp"

namespace Graphics {

struct Sphere {
  glm::vec3 center;
  float radius;
};

struct Spheres {

  Spheres();
  void draw(const Camera & camera);

  void clear();
  void append(const Sphere & sphere);
  void append(const std::vector< Sphere > & more_spheres);
  void append(const std::vector< Sphere > & more_spheres, const std::vector < rgbcolor > & more_colors);

  void set_color(rgbcolor c);
  void set_light(glm::vec3 direction, float intensity);

  auto size() { return data.size(); }

 private:
  bool dirty;
  GLuint vao;
  GLuint color_vbo;
  GLuint sphere_vbo;
  GLuint instance_vbo; 
  GLuint instance_ebo;

  ShaderProgram program;

  rgbcolor color;

  std::vector< Sphere > data;
  std::vector< rgbcolor > colors;

  glm::vec4 light;

};

}