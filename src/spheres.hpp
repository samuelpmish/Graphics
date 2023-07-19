#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "RGBColor.hpp"

struct Sphere {
  glm::vec3 center;
  float radius;
};

struct Spheres {

  Spheres();
  void draw(const Camera & camera);

  void clear();
  void append(const std::vector< Sphere > & more_spheres);
  void append(const std::vector< Sphere > & more_spheres, const std::vector < rgbcolor > & more_colors);

  void set_color(rgbcolor c);

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

};