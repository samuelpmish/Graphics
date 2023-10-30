#include "patches.hpp"

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

//uniform vec4 light;

in vec3 vert;
in vec4 rgba;

out vertexData {
  vec3 position;
  vec4 color;
} outData;

void main() {
  outData.position = vert;
  outData.color = rgba;
}
)vert");

static const std::string frag_shader(R"frag(
#version 150

in fragData{
  vec4 color;
} inData;

out vec4 frag_color;

void main() {
  //frag_color = inData.color;
  frag_color = vec4(1.0, 1.0, 0.0, 1.0);
}
)frag");

std::string to_string(PatchType type) {
  switch (type) {
    case PatchType::TRI6: return "Tri6";
    case PatchType::QUAD4: return "Quad4";
    case PatchType::QUAD8: return "Quad8";
    case PatchType::QUAD9: return "Quad9";
  }
}

int vertices_per_patch(PatchType type) {
  switch (type) {
    case PatchType::TRI6: return 6;
    case PatchType::QUAD4: return 4;
    case PatchType::QUAD8: return 8;
    case PatchType::QUAD9: return 9;
  }
}

extern const std::string tri6_tcs_shader;
extern const std::string tri6_tes_shader;

extern const std::string quad4_tcs_shader;
extern const std::string quad4_tes_shader;

extern const std::string quad8_tcs_shader;
extern const std::string quad8_tes_shader;

extern const std::string quad9_tcs_shader;
extern const std::string quad9_tes_shader;

static constexpr PatchType patch_types[4] = {
  PatchType::TRI6, PatchType::QUAD4, PatchType::QUAD8, PatchType::QUAD9 
};

Patches::Patches() : groups{
    {std::vector<Shader>{
      Shader::fromString(vert_shader, GL_VERTEX_SHADER),
      Shader::fromString(quad4_tcs_shader, GL_TESS_CONTROL_SHADER),
      Shader::fromString(quad4_tes_shader, GL_TESS_EVALUATION_SHADER),
      Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
    }},
    {std::vector<Shader>{
      Shader::fromString(vert_shader, GL_VERTEX_SHADER),
      Shader::fromString(quad4_tcs_shader, GL_TESS_CONTROL_SHADER),
      Shader::fromString(quad4_tes_shader, GL_TESS_EVALUATION_SHADER),
      Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
    }},
    {std::vector<Shader>{
      Shader::fromString(vert_shader, GL_VERTEX_SHADER),
      Shader::fromString(quad4_tcs_shader, GL_TESS_CONTROL_SHADER),
      Shader::fromString(quad4_tes_shader, GL_TESS_EVALUATION_SHADER),
      Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
    }},
    {std::vector<Shader>{
      Shader::fromString(vert_shader, GL_VERTEX_SHADER),
      Shader::fromString(quad4_tcs_shader, GL_TESS_CONTROL_SHADER),
      Shader::fromString(quad4_tes_shader, GL_TESS_EVALUATION_SHADER),
      Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)
    }}
  },
  color{255, 255, 255, 255},
  light(0.721995, 0.618853, 0.309426, 0.0) {

  for (auto type : patch_types) {

    auto & g = groups[type];

    g.dirty = true;

    glGenVertexArrays(1, &g.vao);
    glBindVertexArray(g.vao);

    glGenBuffers(1, &g.position_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g.position_vbo);
    g.program.setAttribute("vert", 3, 12, 0);

    glGenBuffers(1, &g.color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g.color_vbo);
    g.program.setAttribute("rgba", 4, 4, 0, GL_TRUE, GL_UNSIGNED_BYTE);

    glCheckError(__FILE__, __LINE__);

  }

}

void Patches::append(const Quad4 & quad) {
  auto & g = groups[PatchType::QUAD4];
  for (auto x : quad) {
    g.positions.push_back(quad[0]);
    g.colors.push_back(color);
  }
  g.dirty = true;
}

void Patches::clear() {
  for (auto & g : groups) {
    g.positions.clear();
    g.colors.clear();
    g.dirty = true;
  }
}

void Patches::set_light(glm::vec3 direction, float intensity) {
  auto unit_direction = normalize(direction);
  light[0] = unit_direction[0];
  light[1] = unit_direction[1];
  light[2] = unit_direction[2];
  light[3] = glm::clamp(intensity, 0.0f, 1.0f);
}

void Patches::set_color(rgbcolor c) {
  color = c;
}

void Patches::draw(const Camera & camera) {

  for (PatchType type : patch_types) {

    auto & g = groups[type];

    g.program.use();

    g.program.setUniform("proj", camera.matrix());
    glCheckError(__FILE__, __LINE__);

    //g.program.setUniform("light", light);
    //glCheckError(__FILE__, __LINE__);

    glBindVertexArray(g.vao);
    if (g.dirty) {
      glBindBuffer(GL_ARRAY_BUFFER, g.position_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * g.positions.size(), &g.positions[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, g.color_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(rgbcolor) * g.colors.size(), &g.colors[0], GL_STATIC_DRAW);
      g.dirty = false;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glPatchParameteri(GL_PATCH_VERTICES, vertices_per_patch(type));
    glDrawArrays(GL_PATCHES, 0, g.positions.size());

    g.program.unuse();

  }

}

}