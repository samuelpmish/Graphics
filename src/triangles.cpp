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

#if 0
// tessellation pattern for tri6 (w/ subdivisions == 2)
//
// o
// * *
// *   *
// *     *
// * * * * *
// * *     * *
// *   *   *   *
// *     * *     *
// o * * * * * * * o
// * *     * *     * *
// *   *   *   *   *   *
// *     * *     * *     *
// * * * * * * * * * * * * *         
// * *     * *     * *     * *
// *   *   *   *   *   *   *   *
// *     * *     * *     * *     *
// o * * * * * * * o * * * * * * * o
//
//
// tessellation pattern for Quad8, Quad9 (w/ subdivisions == 2)
//
// o * * * * * * * o * * * * * * * o
// * *     * *     *     * *     * *
// *   *   *   *   *   *   *   *   *
// *     * *     * * *     * *     *
// * * * * * * * * * * * * * * * * *
// * *     * *     *     * *     * *
// *   *   *   *   *   *   *   *   *
// *     * *     * * *     * *     *
// o * * * * * * *(o)* * * * * * * o
// *     * *     * * *     * *     *
// *   *   *   *   *   *   *   *   *
// * *     * *     *     * *     * *
// * * * * * * * * * * * * * * * * *
// *     * *     * * *     * *     *
// *   *   *   *   *   *   *   *   *
// * *     * *     *     * *     * *
// o * * * * * * * o * * * * * * * o

template < typename geom >
glm::vec3 surface_normal(const geom & p, float xi[2]) {

  if constexpr (std::is_same<geom, Quad4>::value) {
    glm::vec3 dx_dxi = 0.25f * ((p[0]-p[1]) * (xi[1]-1.0f) + (p[2]-p[3]) * (xi[1]+1.0f));
    glm::vec3 dx_deta = 0.25f * ((p[0]-p[3]) * (xi[0]-1.0f) + (p[2]-p[1]) * (xi[0]+1.0f));
    return glm::normalize(glm::cross(dx_dxi, dx_deta));
  }

  if constexpr (std::is_same<geom, Tri6>::value) {
    glm::vec3 dx_dxi = p[0] * (4.0f * (xi[0] + xi[1]) - 3) + 
                       p[1] * (4.0f * xi[0] - 1) +
                       p[2] * 0.0f +  
                       p[3] * (4.0f - 8.0f * xi[0] - 4.0f * xi[1]) + 
                       p[4] * 4.0f * xi[1] -
                       p[5] * 4.0f * xi[1];

    glm::vec3 dx_deta = p[0] * (4.0f * (xi[0] + xi[1]) - 3) + 
                        p[1] * 0.0f + 
                        p[2] * (4.0f * xi[1] - 1.0f) - 
                        p[3] * (4.0f * xi[0]) +
                        p[4] * (4.0f * xi[0]) +
                        p[5] * (4.0f - 4.0f * xi[0] - 8.0f * xi[1]);

    return glm::normalize(glm::cross(dx_dxi, dx_deta));
  }

  return {};

}

template < typename geom >
glm::vec3 interpolate(const geom & p, float xi[2]) {
  if constexpr (std::is_same<geom, Quad4>::value) {
    return 0.25f * (p[0] * (1.0f - xi[0]) * (1.0f - xi[1]) +
                    p[1] * (1.0f + xi[0]) * (1.0f - xi[1]) +
                    p[2] * (1.0f + xi[0]) * (1.0f + xi[1]) +
                    p[3] * (1.0f - xi[0]) * (1.0f + xi[1]));
  }

  if constexpr (std::is_same<geom, Tri6>::value) {
    return p[0] * (1.0f - xi[0] - xi[1]) * (1.0f - 2 * xi[0] - 2 * xi[1]) +
           p[1] * xi[0] * (2.0f * xi[0] - 1)                              +
           p[2] * xi[1] * (2.0f * xi[1] - 1)                              +
           p[3] * 4.0f * xi[0] * (1 - xi[0] - xi[1])                      +
           p[4] * 4.0f * xi[0] * xi[1]                                    +
           p[5] * 4.0f * xi[1] * (1 - xi[0] - xi[1]);
  }

  return {};
}

template < typename geom >
void Triangles::tessellate(const geom & p) {

  const int k = 2 * subdivision;
  const float dxi = 1.0f / k;

  float xi[2];
  glm::vec3 x[k+1][k+1];
  glm::vec3 n[k+1][k+1];

  if constexpr (std::is_same<geom, Tri6>::value) {
    for (int j = 0; j < k+1; j++) {
      xi[1] = j * dxi;
      for (int i = 0; i < k+1-j; i++) {
        xi[0] = i * dxi;
        x[i][j] = interpolate(p, xi);
        n[i][j] = surface_normal(p, xi);
      }
    }

    for (int j = 0; j < k; j++) {
      for (int i = 0; i < k - j; i++) {
        colors.push_back({color, color, color});
        normals.push_back({n[i][j], n[i+1][j], n[i][j+1]});
        vertices.push_back({x[i][j], x[i+1][j], x[i][j+1]});
        if ((i+j) < (k-1)) { 
          colors.push_back({color, color, color});
          normals.push_back({n[i+1][j], n[i+1][j+1], n[i][j+1]});
          vertices.push_back({x[i+1][j], x[i+1][j+1], x[i][j+1]});
        }
      }
    }

  }

  if constexpr (std::is_same<geom, Quad4>::value || 
                std::is_same<geom, Quad8>::value || 
                std::is_same<geom, Quad9>::value) {

    for (int i = 0; i < k+1; i++) {
      xi[0] = -1.0f + i * dxi * 2.0f;
      for (int j = 0; j < k+1; j++) {
        xi[1] = -1.0f + j * dxi * 2.0f;
        x[i][j] = interpolate(p, xi);
        n[i][j] = surface_normal(p, xi);
      }
    }

    for (int i = 0; i < k; i++) {
      for (int j = 0; j < k; j++) {
        // j       
        // ^       
        // |       
        // 1 1 0 0 
        // 1 1 0 0 
        // 0 0 1 1
        // 0 0 1 1 --> i
        int offset = (i / k) ^ (j / k);

        // offset = 0   offset = 1
        //  o * * o      o * * o
        //  *   * *      * *   * 
        //  * *   *      *   * *
        //  o * * o      o * * o 
        colors.push_back({color, color, color});
        normals.push_back({n[i][j], n[i+1][j], n[i+1-offset][j+1]});
        vertices.push_back({x[i][j], x[i+1][j], x[i+1-offset][j+1]});

        colors.push_back({color, color, color});
        normals.push_back({n[i+1][j+1], n[i][j+1], n[i+offset][j]});
        vertices.push_back({x[i+1][j+1], x[i][j+1], x[i+offset][j]});
      }
    }

  }

}
#endif

template < typename tri_t >
Tri3 normalVectors(const tri_t & triangle) {
  auto n = normalize(cross(triangle[1] - triangle[0],
                           triangle[2] - triangle[0]));
  return {n, n, n};
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
  program.setAttribute("vert", 3, 12, 0);

  glGenBuffers(1, &normal_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  program.setAttribute("normal", 3, 12, 0);

  glGenBuffers(1, &color_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  program.setAttribute("rgba", 4, 4, 0, GL_TRUE, GL_UNSIGNED_BYTE);

  glCheckError(__FILE__, __LINE__);

}

void Triangles::clear() {
  vertices.clear();
  normals.clear();
  dirty = true;
}

void Triangles::append(const std::array< glm::vec3, 3 > & tri) {
  vertices.push_back(tri);
  normals.push_back(normalVectors(tri));
  colors.push_back({color, color, color});
  dirty = true;
}

//void Triangles::append(const std::vector< Triangle > & more_triangles) {
//  data.reserve(data.size() + more_triangles.size());
//  for (uint32_t i = 0; i < more_triangles.size(); i++) {
//    data.push_back(addColor(more_triangles[i], color));
//    normals.push_back(normalVectors(more_triangles[i]));
//  }
//  dirty = true;
//}
//
//void Triangles::append(const std::vector< Triangle > & more_triangles,
//                       const std::vector< rgbcolor > & more_colors) {
//  data.reserve(data.size() + more_triangles.size());
//  for (uint32_t i = 0; i < more_triangles.size(); i++) {
//    data.push_back(addColor(more_triangles[i], more_colors[i]));
//    normals.push_back(normalVectors(more_triangles[i]));
//  }
//  dirty = true;
//}

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tri3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Tri3) * normals.size(), &normals[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color3) * colors.size(), &colors[0], GL_STATIC_DRAW);
    dirty = false;
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glDrawArrays(GL_TRIANGLES, 0, vertices.size() * 3);

  program.unuse();

}

}