#include "spheres.hpp"

#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

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

in vec4 sphere;

in vec3 corners;

out vec3 sphere_center;
out float sphere_radius;
out vec3 position;

uniform mat4 proj;

void main() {
  sphere_center = sphere.xyz;
  sphere_radius = sphere.w;
  position = sphere.w * corners + sphere.xyz;
  gl_Position = proj * vec4(position, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

//in fragData {
//  vec4 color;
//  vec3 position;
//} inData;

in vec3 sphere_center;
in float sphere_radius;
in vec3 position;

out vec4 frag_color;

uniform vec3 camera_position;
uniform mat4 proj;

void main() {

  frag_color = vec4(1,0,0,1);

  float R = sphere_radius;

  // relative sphere location
  vec3 S = sphere_center - camera_position;

  // ray direction
  vec3 D = normalize(position - camera_position);
  
  // quadratic equation coefficients (A is always 1)
  float B = -2.0 * dot(D, S);
  float C = dot(S, S) - (R * R);
  
  float discr = (B * B) - (4 * C);
  float delta = fwidth(discr);
  float alpha = smoothstep(0, delta, discr);
  float depth = 1.0 - smoothstep(-delta * 0.5, delta * 0.5, discr);
  frag_color.a *= alpha;

  float t = (-B - sqrt(max(discr, 0.0))) / 2.0;

  vec3 p = camera_position + D * t;

  vec4 clip = proj * vec4(p, 1.0);
  float ndcDepth = clip.z / clip.w;
  gl_FragDepth = (((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0) + depth;

  frag_color = vec4(1.0, 1.0, 1.0, 1.0);

}
)frag");

Spheres::Spheres() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }) {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &cube_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

  glGenBuffers(1, &cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
  program.setAttribute("corners", 3, sizeof(glm::vec3), 0);

  glGenBuffers(1, &sphere_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
  program.setAttribute("sphere", 4, sizeof(glm::vec4), 0);
  glVertexAttribDivisor(program.attribute("sphere"), 1);

}

void Spheres::clear() {
  data.clear();
  dirty = true;
}

void Spheres::append(const std::vector< Sphere > & more_spheres) {
  data.reserve(data.size() + more_spheres.size());
  data.insert(data.end(), more_spheres.begin(), more_spheres.end());  
  dirty = true;
}

void Spheres::draw(const Camera & camera) {

  program.use();

  auto proj = camera.matrix();

  //auto v = camera.up();
  //auto h = glm::normalize(glm::cross(v, camera.m_pos - camera.m_focus));
  glUniformMatrix4fv(program.uniform("proj"), 1, GL_FALSE, glm::value_ptr(camera.matrix()));

  program.setUniform("camera_position", camera.pos());

  glBindVertexArray(vao);
  if (dirty) {
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere) * data.size(), &data[0], GL_STATIC_DRAW);
    dirty = false;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElementsInstanced(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0, data.size());

  program.unuse();

}