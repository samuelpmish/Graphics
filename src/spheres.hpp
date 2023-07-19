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
  GLuint sphere_vbo;
  GLuint instance_vbo; 
  GLuint instance_ebo;

  ShaderProgram program;

  std::vector< Sphere > data;

};