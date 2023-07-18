#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"

struct Sphere {
  glm::vec3 center;
  float radius;
};

struct Spheres {

  Spheres();
  void draw(const Camera & camera);

  void clear();
  void append(const std::vector< Sphere > & more_spheres);

 private:
  bool dirty;
  GLuint vao;
  GLuint cube_vbo; 
  GLuint cube_ebo;
  GLuint sphere_vbo;

  ShaderProgram program;

  std::vector< Sphere > data;

};