#include "cylinders.hpp"

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "glError.hpp"

namespace Graphics {

static constexpr int angular_divisions = 16;

static const std::vector< glm::vec3 > cylinder_vertices = [](){
  std::vector< glm::vec3 > vertices(2 * (angular_divisions + 1));
  for (int i = 0; i <= angular_divisions; i++) {
    float theta = i * (2.0 * M_PI / angular_divisions);
    float c = cos(theta);
    float s = sin(theta);
    vertices[2*i+0] = {c, s, 0.0};
    vertices[2*i+1] = {c, s, 1.0};
  }
  return vertices;
}();

const std::string vert_shader(R"vert(
#version 400

in vec4 cyl_start;
in vec4 cyl_end;
in vec3 corners;
in vec4 rgba;

out vec3 normal;
out vec4 cylinder_color;

uniform mat4 proj;

void main() {
  cylinder_color = rgba;

  vec3 e3 = cyl_end.xyz - cyl_start.xyz;
  //vec3 e1 = vec3(1,0,0);
  vec3 e1 = cross(e3, vec3(0,0,1));
  if (length(e1) < 1.0e-5) {
    e1 = vec3(1,0,0);
  } else {
    e1 = normalize(e1);
  }
  vec3 e2 = normalize(cross(e3, e1));
  float r = cyl_start.w + corners.z * (cyl_end.w - cyl_start.w);

  normal = normalize(corners.x * e1 + corners.y * e2);
  gl_Position = proj * vec4(cyl_start.xyz + r * corners.x * e1 + r * corners.y * e2 + corners.z * e3, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

in vec3 normal;
in vec4 cylinder_color;

out vec4 frag_color;

uniform vec4 light;

void main() {
  frag_color = cylinder_color;
  if (light.w != 0) {
    float ambient = 1.0 - light.w;
    float diffuse = clamp(dot(normal,light.xyz), 0.0, 1.0) * light.w;
    frag_color *= ambient + diffuse;
  }
}
)frag");

Cylinders::Cylinders() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }), 
  color{255, 255, 255, 255},
  light(0.721995, 0.618853, 0.309426, 0.0) {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &instance_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cylinder_vertices.size(), &cylinder_vertices[0], GL_STATIC_DRAW);
  program.setAttribute("corners", 3, sizeof(glm::vec3), 0);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &cylinder_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);

  program.setAttribute("cyl_start", 4, 2 * sizeof(glm::vec4), 0);
  glVertexAttribDivisor(program.attribute("cyl_start"), 1);

  program.setAttribute("cyl_end", 4, 2 * sizeof(glm::vec4), 16);
  glVertexAttribDivisor(program.attribute("cyl_end"), 1);

  glGenBuffers(1, &color_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  program.setAttribute("rgba", 4, sizeof(rgbcolor), 0, GL_TRUE, GL_UNSIGNED_BYTE);
  glVertexAttribDivisor(program.attribute("rgba"), 1);

}

void Cylinders::clear() {
  data.clear();
  colors.clear();
  dirty = true;
}

void Cylinders::append(const Cylinder & cylinder) {
  data.push_back(cylinder);
  colors.push_back(color);
  dirty = true;
}

void Cylinders::append(const std::vector< Cylinder > & more_cylinders) {
  data.reserve(data.size() + more_cylinders.size());
  data.insert(data.end(), more_cylinders.begin(), more_cylinders.end());  

  colors.insert(colors.end(), more_cylinders.size(), color);
  dirty = true;
}

void Cylinders::append(const std::vector< Cylinder > & more_cylinders,
                     const std::vector< rgbcolor > & more_colors) {
  data.reserve(data.size() + more_cylinders.size());
  data.insert(data.end(), more_cylinders.begin(), more_cylinders.end());  

  colors.reserve(colors.size() + more_colors.size());
  colors.insert(colors.end(), more_colors.begin(), more_colors.end());  
  dirty = true;
}

void Cylinders::set_light(glm::vec3 direction, float intensity) {
  auto unit_direction = normalize(direction);
  light[0] = unit_direction[0];
  light[1] = unit_direction[1];
  light[2] = unit_direction[2];
  light[3] = glm::clamp(intensity, 0.0f, 1.0f);
}

void Cylinders::set_color(rgbcolor c) {
  color = c;
}

void Cylinders::draw(const Camera & camera) {

  program.use();

  program.setUniform("light", light);
  program.setUniform("proj", camera.matrix());
  glCheckError(__FILE__, __LINE__);

  glBindVertexArray(vao);
  if (dirty) {
    if (colors.size() != data.size()) {
      std::cout << "error: `Cylinder` buffer sizes are incompatible" << std::endl;
    }

    glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cylinder) * data.size(), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgbcolor) * colors.size(), &colors[0], GL_STATIC_DRAW);
    glCheckError(__FILE__, __LINE__);
    dirty = false;
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE);
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, cylinder_vertices.size(), data.size());
  glCheckError(__FILE__, __LINE__);

  program.unuse();

}

}