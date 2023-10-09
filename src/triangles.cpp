#include "triangles.hpp"

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "glError.hpp"

namespace Graphics {

static const std::string vert_shader(R"vert(
#version 150

uniform mat4 proj;
uniform vec4 light;

in vec3 vert;
in vec4 rgba;
in vec3 normal;

out vec4 triangle_color;

void main() {
  gl_Position = proj * vec4(vert,1);
  triangle_color = rgba;
  if (light.w != 0) {
    float ambient = 1.0 - light.w;
    float diffuse = clamp(dot(normal,light.xyz), 0.0, 1.0) * light.w;
    triangle_color *= ambient + diffuse;
  }
}
)vert");

static const std::string frag_shader(R"frag(
#version 150

in vec4 triangle_color;
out vec4 frag_color;

void main() {
  frag_color = triangle_color;
}
)frag");

Triangle normalVectors(const Triangle & triangle) {
  auto n = normalize(cross(triangle.vertices[1] - triangle.vertices[0],
                           triangle.vertices[2] - triangle.vertices[0]));
  return {n, n, n};
}

TriangleWithColor addColor(const Triangle & triangle, rgbcolor color) {
  return {{{triangle.vertices[0], color}, 
           {triangle.vertices[1], color}, 
           {triangle.vertices[2], color}}};
}

Triangles::Triangles() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }),
  color{255, 255, 255, 255},
  light(0.721995, 0.618853, 0.309426, 0.0) {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &triangle_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
  int stride = 16; // sizeof each vertex
  program.setAttribute("vert", 3, 16, 0);
  program.setAttribute("rgba", 4, 16, 12, GL_TRUE, GL_UNSIGNED_BYTE);

  glGenBuffers(1, &normal_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  program.setAttribute("normal", 3, 12, 0);

  glCheckError(__FILE__, __LINE__);

}

void Triangles::clear() {
  data.clear();
  dirty = true;
}

void Triangles::append(const Triangle & triangle) {
  data.push_back(addColor(triangle, color));
  normals.push_back(normalVectors(triangle));
  dirty = true;
}

void Triangles::append(const std::vector< Triangle > & more_triangles) {
  data.reserve(data.size() + more_triangles.size());
  for (uint32_t i = 0; i < more_triangles.size(); i++) {
    data.push_back(addColor(more_triangles[i], color));
    normals.push_back(normalVectors(more_triangles[i]));
  }
  dirty = true;
}

void Triangles::append(const std::vector< Triangle > & more_triangles,
                       const std::vector< rgbcolor > & more_colors) {
  data.reserve(data.size() + more_triangles.size());
  for (uint32_t i = 0; i < more_triangles.size(); i++) {
    data.push_back(addColor(more_triangles[i], more_colors[i]));
    normals.push_back(normalVectors(more_triangles[i]));
  }
  dirty = true;
}

void Triangles::set_light(glm::vec3 direction, float intensity) {
  auto unit_direction = normalize(direction);
  light[0] = unit_direction[0];
  light[1] = unit_direction[1];
  light[2] = unit_direction[2];
  light[3] = glm::clamp(intensity, 0.0f, 1.0f);
}

void Triangles::set_color(rgbcolor c) {
  color = c;
}

void Triangles::draw(const Camera & camera) {

  program.use();

  program.setUniform("proj", camera.matrix());
  program.setUniform("light", light);
  glCheckError(__FILE__, __LINE__);

  glBindVertexArray(vao);
  if (dirty) {
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleWithColor) * data.size(), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * normals.size(), &normals[0], GL_STATIC_DRAW);
    dirty = false;
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glDrawArrays(GL_TRIANGLES, 0, data.size() * 3);

  program.unuse();

}

}