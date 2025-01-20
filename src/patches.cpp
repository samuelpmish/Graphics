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

static constexpr int VERTEX_COLOR = 0;
static constexpr int PALETTE = 1;

static const std::string vert_shader_color(R"vert(
#version 400

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

static const std::string vert_shader_value(R"vert(
#version 400

in vec3 vert;
in float value;

out vertexData {
  vec3 position;
  float value;
} outData;

void main() {
  outData.position = vert;
  outData.value = value;
}
)vert");

static const std::string frag_shader_color(R"frag(
#version 400

in fragData{
  vec4 color;
} inData;

out vec4 frag_color;

void main() {
  frag_color = inData.color;
}
)frag");

static const std::string frag_shader_value(R"frag(
#version 400

in fragData{
  float value;
} inData;

out vec4 frag_color;

uniform sampler1D palette;
uniform float min_value;
uniform float max_value;
uniform int posterize;

void main() {
  float t = clamp((inData.value - min_value) / (max_value - min_value), 0.0, 1.0);

  if (posterize != 0) {
    t = round(t * posterize) / posterize;
  }

  frag_color = texture(palette, t).rgba;
}
)frag");

std::string to_string(PatchType type) {
  switch (type) {
    case PatchType::TRI6: return "Tri6";
    case PatchType::QUAD4: return "Quad4";
    case PatchType::QUAD8: return "Quad8";
    case PatchType::QUAD9: return "Quad9";
  }
  return {};
}

int vertices_per_patch(PatchType type) {
  switch (type) {
    case PatchType::TRI6: return 6;
    case PatchType::QUAD4: return 4;
    case PatchType::QUAD8: return 8;
    case PatchType::QUAD9: return 9;
  }
  return {};
}

extern const std::string tri6_tcs_shader_color;
extern const std::string tri6_tes_shader_color;

extern const std::string tri6_tcs_shader_value;
extern const std::string tri6_tes_shader_value;

extern const std::string quad4_tcs_shader_color;
extern const std::string quad4_tes_shader_color;

extern const std::string quad8_tcs_shader_color;
extern const std::string quad8_tes_shader_color;

extern const std::string quad9_tcs_shader_color;
extern const std::string quad9_tes_shader_color;

static constexpr PatchType patch_types[4] = {
  PatchType::TRI6, PatchType::QUAD4, PatchType::QUAD8, PatchType::QUAD9 
};

std::vector< std::string > tessellated_tri6_shaders_color() {
  return {vert_shader_color, tri6_tcs_shader_color,  tri6_tes_shader_color, frag_shader_color};
}

std::vector< std::string > tessellated_tri6_shaders_value() {
  return {vert_shader_value, tri6_tcs_shader_value,  tri6_tes_shader_value, frag_shader_value};
}

std::vector< std::string > tessellated_quad4_shaders_color() {
  return {vert_shader_color, quad4_tcs_shader_color,  quad4_tes_shader_color, frag_shader_color};
}

std::vector< std::string > tessellated_quad8_shaders_color() {
  return {vert_shader_color, quad8_tcs_shader_color,  quad8_tes_shader_color, frag_shader_color};
}

std::vector< std::string > tessellated_quad9_shaders_color() {
  return {vert_shader_color, quad9_tcs_shader_color,  quad9_tes_shader_color, frag_shader_color};
}

Patches::RenderGroup::RenderGroup(const std::vector<std::string> & shaders, bool palette) : 
  program({
    Shader::fromString(shaders[0], GL_VERTEX_SHADER),
    Shader::fromString(shaders[1], GL_TESS_CONTROL_SHADER),
    Shader::fromString(shaders[2], GL_TESS_EVALUATION_SHADER),
    Shader::fromString(shaders[3], GL_FRAGMENT_SHADER)
  }) {

  dirty = false;
  subdivision = 3;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glCheckError(__FILE__, __LINE__);

  glGenBuffers(1, &position_vbo);
  glCheckError(__FILE__, __LINE__);

  glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
  program.setAttribute("vert", 3, 12, 0);
  glCheckError(__FILE__, __LINE__);

  if (palette) {
    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    program.setAttribute("value", 1, 4, 0);
    glCheckError(__FILE__, __LINE__);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_1D, texture);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
 } else {
    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    program.setAttribute("rgba", 4, 4, 0, GL_TRUE, GL_UNSIGNED_BYTE);
    glCheckError(__FILE__, __LINE__);
  }

}

Patches::Patches() : groups{
  {
    {tessellated_tri6_shaders_color()},
    {tessellated_quad4_shaders_color()},
    {tessellated_quad8_shaders_color()},
    {tessellated_quad9_shaders_color()}
  }, {
    {tessellated_tri6_shaders_value(), true},
    {tessellated_quad4_shaders_color()},
    {tessellated_quad8_shaders_color()},
    {tessellated_quad9_shaders_color()}
  }
},
  color{255, 255, 255, 255},
  light(0.721995, 0.618853, 0.309426, 0.0),
  posterize{0},
  interval{0.0, 1.0} {

  palette = {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}};
}

////////////////////////////////////////

void Patches::append(const Quad4 & quad) {
  auto & g = groups[VERTEX_COLOR][PatchType::QUAD4];
  for (auto x : quad) {
    g.positions.push_back(x);
    g.colors.push_back(color);
  }
  g.dirty = true;
}

void Patches::append(const Tri6 & tri) {
  auto & g = groups[VERTEX_COLOR][PatchType::TRI6];
  for (auto x : tri) {
    g.positions.push_back(x);
    g.colors.push_back(color);
  }
  g.dirty = true;
}

void Patches::append(const Quad8 & quad) {
  auto & g = groups[VERTEX_COLOR][PatchType::QUAD8];
  for (auto x : quad) {
    g.positions.push_back(x);
    g.colors.push_back(color);
  }
  g.dirty = true;
}

void Patches::append(const Quad9 & quad) {
  auto & g = groups[VERTEX_COLOR][PatchType::QUAD9];
  for (auto x : quad) {
    g.positions.push_back(x);
    g.colors.push_back(color);
  }
  g.dirty = true;
}

////////////////////////////////////////

void Patches::append(const Quad4v & quad) {
  auto & g = groups[PALETTE][PatchType::QUAD4];
  for (auto x : quad) {
    g.positions.push_back({x[0], x[1], x[2]});
    g.values.push_back(x[3]);
  }
  g.dirty = true;
}

void Patches::append(const Tri6v & tri) {
  auto & g = groups[PALETTE][PatchType::TRI6];
  for (auto x : tri) {
    g.positions.push_back({x[0], x[1], x[2]});
    g.values.push_back(x[3]);
  }
  g.dirty = true;
}

void Patches::append(const Quad8v & quad) {
  auto & g = groups[PALETTE][PatchType::QUAD8];
  for (auto x : quad) {
    g.positions.push_back({x[0], x[1], x[2]});
    g.values.push_back(x[3]);
  }
  g.dirty = true;
}

void Patches::append(const Quad9v & quad) {
  auto & g = groups[PALETTE][PatchType::QUAD9];
  for (auto x : quad) {
    g.positions.push_back({x[0], x[1], x[2]});
    g.values.push_back(x[3]);
  }
  g.dirty = true;
}

////////////////////////////////////////

void Patches::clear() {
  for (auto & g : groups[VERTEX_COLOR]) {
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

  for (int coloring : {VERTEX_COLOR, PALETTE}) {
    for (PatchType type : patch_types) {

      auto & g = groups[coloring][type];

      if (g.positions.size() == 0) continue;

      g.program.use();

      if (coloring == PALETTE) {
        g.program.setUniform("min_value", interval[0]);
        g.program.setUniform("max_value", interval[1]);
        g.program.setUniform("posterize", posterize);
      }

      g.program.setUniform("subdivision", g.subdivision);
      glCheckError(__FILE__, __LINE__);

      g.program.setUniform("proj", camera.matrix());
      glCheckError(__FILE__, __LINE__);

      //g.program.setUniform("light", light);
      //glCheckError(__FILE__, __LINE__);

      glBindVertexArray(g.vao);
      if (g.dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, g.position_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * g.positions.size(), &g.positions[0], GL_STATIC_DRAW);

        if (g.values.size() == 0) {

          // color by vertex
          glBindBuffer(GL_ARRAY_BUFFER, g.color_vbo);
          glBufferData(GL_ARRAY_BUFFER, sizeof(rgbcolor) * g.colors.size(), &g.colors[0], GL_STATIC_DRAW);

        } else {

          // color by value
          glBindBuffer(GL_ARRAY_BUFFER, g.color_vbo);
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g.values.size(), &g.values[0], GL_STATIC_DRAW);

          glBindTexture(GL_TEXTURE_1D, g.texture);
          glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, palette.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &palette[0]);

        }

        g.dirty = false;
      }

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

      glDisable(GL_CULL_FACE);
      //glCullFace(GL_BACK);

      glPatchParameteri(GL_PATCH_VERTICES, vertices_per_patch(type));
      glDrawArrays(GL_PATCHES, 0, g.positions.size());

      g.program.unuse();

    }
  }

}

}