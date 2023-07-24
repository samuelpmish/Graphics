#include "spheres.hpp"

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

static float instance_vertices[8] = {
	-1.0f, -1.0f,
	 1.0f, -1.0f,
	 1.0f, +1.0f,
	-1.0f, +1.0f,
};

static uint32_t instance_triangles[4] = {1, 2, 0, 3};

const std::string vert_shader(R"vert(
#version 400

in vec4 sphere;
in vec2 corners;
in vec4 rgba;

out vec4 sphere_color;
out vec3 sphere_center;
out float sphere_radius;
out vec3 position;

uniform mat4 proj;
uniform vec3 up;
uniform vec3 camera_position;

void main() {
  sphere_color = rgba;
  sphere_center = sphere.xyz;
  sphere_radius = sphere.w;
  vec3 e1 = normalize(cross(sphere_center - camera_position, up));
  vec3 e2 = normalize(cross(e1, sphere_center - camera_position));
  position = 1.05 * sphere.w * (corners.x * e1 + corners.y * e2) + sphere.xyz;
  gl_Position = proj * vec4(position, 1);
}
)vert");

const std::string frag_shader(R"frag(
#version 400

in vec3 sphere_center;
in float sphere_radius;
in vec3 position;

in vec4 sphere_color;

out vec4 frag_color;

uniform vec4 light;
uniform vec3 camera_position;
uniform mat4 proj;

void main() {

  frag_color = sphere_color;

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

  if (t > 0 && light.w != 0) {
    vec3 normal = normalize(p - sphere_center);
    float ambient = 1.0 - light.w;
    float diffuse = clamp(dot(normal,light.xyz), 0.0, 1.0) * light.w;
    frag_color *= ambient + diffuse;
  }

  vec4 clip = proj * vec4(p, 1.0);
  float ndcDepth = clip.z / clip.w;
  gl_FragDepth = (((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0) + depth;

}
)frag");

Spheres::Spheres() : program({
    Shader::fromString(vert_shader, GL_VERTEX_SHADER),
    Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
  }),
  color{255, 255, 255, 255},
  light(0.721995, 0.618853, 0.309426, 0.0) {

  dirty = true;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &instance_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(instance_triangles), instance_triangles, GL_STATIC_DRAW);

  glGenBuffers(1, &instance_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(instance_vertices), instance_vertices, GL_STATIC_DRAW);
  program.setAttribute("corners", 2, sizeof(glm::vec2), 0);

  glGenBuffers(1, &sphere_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
  program.setAttribute("sphere", 4, sizeof(glm::vec4), 0);
  glVertexAttribDivisor(program.attribute("sphere"), 1);

  glGenBuffers(1, &color_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  program.setAttribute("rgba", 4, sizeof(rgbcolor), 0, GL_TRUE, GL_UNSIGNED_BYTE);
  glVertexAttribDivisor(program.attribute("rgba"), 1);

}

void Spheres::clear() {
  data.clear();
  colors.clear();
  dirty = true;
}

void Spheres::append(const Sphere & sphere) {
  data.push_back(sphere);
  colors.push_back(color);
  dirty = true;
}

void Spheres::append(const std::vector< Sphere > & more_spheres) {
  data.reserve(data.size() + more_spheres.size());
  data.insert(data.end(), more_spheres.begin(), more_spheres.end());  

  colors.insert(colors.end(), more_spheres.size(), color);
  dirty = true;
}

void Spheres::append(const std::vector< Sphere > & more_spheres,
                     const std::vector< rgbcolor > & more_colors) {
  data.reserve(data.size() + more_spheres.size());
  data.insert(data.end(), more_spheres.begin(), more_spheres.end());  

  colors.reserve(colors.size() + more_colors.size());
  colors.insert(colors.end(), more_colors.begin(), more_colors.end());  
  dirty = true;
}

void Spheres::set_light(glm::vec3 direction, float intensity) {
  auto unit_direction = normalize(direction);
  light[0] = unit_direction[0];
  light[1] = unit_direction[1];
  light[2] = unit_direction[2];
  light[3] = glm::clamp(intensity, 0.0f, 1.0f);
}

void Spheres::set_color(rgbcolor c) {
  color = c;
}

void Spheres::draw(const Camera & camera) {

  program.use();

  program.setUniform("light", light);
  program.setUniform("up", camera.up());
  program.setUniform("proj", camera.matrix());
  program.setUniform("camera_position", camera.pos());

  glBindVertexArray(vao);
  if (dirty) {
    if (colors.size() != data.size()) {
      std::cout << "error: `Sphere` buffer sizes are incompatible" << std::endl;
    }

    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Sphere) * data.size(), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgbcolor) * colors.size(), &colors[0], GL_STATIC_DRAW);
    dirty = false;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance_ebo);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0, data.size());

  program.unuse();

}