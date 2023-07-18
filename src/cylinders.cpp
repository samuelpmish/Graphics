#include "cylinders.hpp"

#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "glError.hpp"

static float cube_vertices[24] = {
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

static uint32_t cube_indices[36] = {
	1, 2, 6, 6, 5, 1, // Right
	0, 4, 7, 7, 3, 0, // Left
	4, 5, 6, 6, 7, 4, // Top
	0, 3, 2, 2, 1, 0, // Bottom
	0, 1, 5, 5, 4, 0, // Back
	3, 7, 6, 6, 2, 3  // Front
};

const std::string vert_shader(R"vert(
#version 400

in vec4 cyl_start;
in vec4 cyl_end;

in vec3 corners;

//out cylinderData {
//  vec3 center;
//  float radius;
//} outData;

uniform mat4 proj;

void main() {
  //outData.center = cylinder.xyz;
  //outData.radius = cylinder.w;

  vec3 mid = 0.5 * (cyl_end.xyz + cyl_start.xyz);
  vec3 e1 = 0.5 * (cyl_end.xyz - cyl_start.xyz);
  vec3 e2 = normalize(cross(e1, vec3(0,0,1)));
  vec3 e3 = normalize(cross(e1, e2));
  float r = 0.5 * ((1 - corners.x) * cyl_start.w + (1 + corners.x) * cyl_end.w);

  gl_Position = proj * vec4(mid - corners.x * e1 + r * corners.y * e2 + r * corners.z * e3, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

//in fragData {
//  vec4 color;
//  vec3 position;
//} inData;

//in cylinderData {
//  vec3 center;
//  float radius;
//  vec4 color;
//} cylinder;

out vec4 frag_color;

uniform vec3 camera_position;
uniform mat4 proj;

void main() {

//  frag_color = inData.color;
//
//  float R = cylinder.radius;
//
//  // relative cylinder location
//  vec3 S = cylinder.center - camera_position;
//
//  // ray direction
//  vec3 D = normalize(inData.position - camera_position);
//  
//  // quadratic equation coefficients (A is always 1)
//  float B = -2.0 * dot(D, S);
//  float C = dot(S, S) - (R * R);
//  
//  float discr = (B * B) - (4 * C);
//  float delta = fwidth(discr);
//  float alpha = smoothstep(0, delta, discr);
//  float depth = 1.0 - smoothstep(-delta * 0.5, delta * 0.5, discr);
//  frag_color.a *= alpha;
//
//  float t = (-B - sqrt(max(discr, 0.0))) / 2.0;
//
//  vec3 p = camera_position + D * t;
//
//  vec4 clip = proj * vec4(p, 1.0);
//  float ndcDepth = clip.z / clip.w;
//  gl_FragDepth = (((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0) + depth;

  frag_color = vec4(1.0, 1.0, 1.0, 1.0);

}
)frag");

Cylinders::Cylinders() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }) {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &cube_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
  program.setAttribute("corners", 3, sizeof(glm::vec3), 0);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &cylinder_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
  program.setAttribute("cyl_start", 4, 2 * sizeof(glm::vec4), 0);
  glCheckError(__FILE__, __LINE__);

  glVertexAttribDivisor(program.attribute("cyl_start"), 1);
  glCheckError(__FILE__, __LINE__);

  program.setAttribute("cyl_end", 4, 2 * sizeof(glm::vec4), 16);
  glCheckError(__FILE__, __LINE__);

  glVertexAttribDivisor(program.attribute("cyl_end"), 1);
  glCheckError(__FILE__, __LINE__);

}

void Cylinders::clear() {
  data.clear();
  dirty = true;
}

void Cylinders::append(const std::vector< Cylinder > & more_cylinders) {
  data.reserve(data.size() + more_cylinders.size());
  data.insert(data.end(), more_cylinders.begin(), more_cylinders.end());  
  dirty = true;
}

void Cylinders::draw(const Camera & camera) {

  program.use();

  auto proj = camera.matrix();

  //auto v = camera.up();
  //auto h = glm::normalize(glm::cross(v, camera.m_pos - camera.m_focus));
  glUniformMatrix4fv(program.uniform("proj"), 1, GL_FALSE, glm::value_ptr(proj));
  glCheckError(__FILE__, __LINE__);

  glBindVertexArray(vao);
  if (dirty) {
    glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cylinder) * data.size(), &data[0], GL_STATIC_DRAW);
    glCheckError(__FILE__, __LINE__);
    dirty = false;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElementsInstanced(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0, data.size());
  glCheckError(__FILE__, __LINE__);

  program.unuse();

}