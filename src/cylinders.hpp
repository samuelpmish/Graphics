#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"
#include "spheres.hpp"

struct Cylinder {
  Sphere endpoints[2];
};

struct Cylinders {

  Cylinders();
  void draw(const Camera & camera);

  void clear();
  void append(const std::vector< Cylinder > & more_cylinders);

 private:
  bool dirty;
  GLuint vao;
  GLuint cube_vbo; 
  GLuint cube_ebo;
  GLuint cylinder_vbo;

  ShaderProgram program;

  std::vector< Cylinder > data;

};