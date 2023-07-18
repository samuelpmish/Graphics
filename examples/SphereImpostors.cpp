#include <array>
#include <vector>
#include <random>
#include <iostream>

#include "glError.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Application.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Canvas : public Application {
 public:
  Canvas();

  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_motion_callback(GLFWwindow* window, double xpos, double ypos);
  void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  void update_camera_position();

  void generate_spheres();

 protected:
  virtual void loop();

 private:
  Camera camera;

  float camera_speed;
  bool keys_down[256];
  double mouse_x, mouse_y;

  bool lmb_down = false;
  bool mmb_down = false;
  bool rmb_down = false;

  Shader vertexShader;
  Shader geometryShader;
  Shader fragmentShader;
  ShaderProgram shaderProgram;

  int32_t n;
  float radius;
  float fov;
  std::vector< glm::vec4 > spheres;

  GLuint vao, vbo;
};

#if defined(__APPLE__)
float DPI_scale = 2;
#else
float DPI_scale = 1;
#endif

const std::string vert_shader(R"vert(
#version 400

in vec4 sphere;

out vertexData {
  vec3 position;
  vec4 color;
  float radius;
} outData;

void main() {
  outData.position = sphere.xyz;
  outData.radius = sphere.w;
  outData.color = vec4(1.0, 0.0, 0.0, 1.0);
}
)vert");

const std::string geom_shader(R"geom(
#version 400

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vertexData {
  vec3 position;
  vec4 color;
  float radius;
} inData[];

out fragData {
  vec4 color;
  vec3 position;
} outData;

out sphereData {
  vec3 center;
  float radius;
} sphere;

uniform mat4 proj;
uniform vec3 camera_position;
uniform vec3 camera_up;

void main (void) {

  float r = inData[0].radius;
  vec3 p = inData[0].position;

  vec3 dh = r * normalize(cross(p - camera_position, camera_up));
  vec3 dv = r * normalize(cross(dh, p - camera_position));

  sphere.center = p;
  sphere.radius = r;

  vec2 uv[4];
  uv[0] = vec2(-1.05,  1.05);
  uv[1] = vec2(-1.05, -1.05);
  uv[2] = vec2( 1.05,  1.05);
  uv[3] = vec2( 1.05, -1.05);

  for (int i = 0; i < 4; i++) {
    outData.color = inData[0].color;
    outData.position = p + uv[i].x * dh + uv[i].y * dv;
    gl_Position = proj * vec4(outData.position, 1); 
    EmitVertex();
  }

}
)geom");

const std::string frag_shader(R"frag(
#version 400

in fragData {
  vec4 color;
  vec3 position;
} inData;

in sphereData {
  vec3 center;
  float radius;
} sphere;

out vec4 frag_color;

uniform vec3 camera_position;
uniform mat4 proj;

void main() {

  frag_color = inData.color;

  float R = sphere.radius;

  // relative sphere location
  vec3 S = sphere.center - camera_position;

  // ray direction
  vec3 D = normalize(inData.position - camera_position);
  
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

}
)frag");

// clang-format off
void key_callback_helper(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto mesher = (Canvas*)glfwGetWindowUserPointer(window);
  mesher->key_callback(window, key, scancode, action, mods);
}

void mouse_scroll_callback_helper(GLFWwindow* window, double xoffset, double yoffset) {
  auto mesher = (Canvas*)glfwGetWindowUserPointer(window);
  mesher->mouse_scroll_callback(window, xoffset, yoffset);
}

void mouse_motion_callback_helper(GLFWwindow* window, double xpos, double ypos) {
  auto mesher = (Canvas*)glfwGetWindowUserPointer(window);
  mesher->mouse_motion_callback(window, xpos, ypos);
}

void mouse_button_callback_helper(GLFWwindow* window, int button, int action, int mods) {
  auto mesher = (Canvas*)glfwGetWindowUserPointer(window);
  mesher->mouse_button_callback(window, button, action, mods);
}
// clang-format on

void Canvas::key_callback(GLFWwindow* window,
                          int key,
                          int scancode,
                          int action,
                          int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  // clang-format off
  if (key == GLFW_KEY_W){ keys_down[uint8_t('w')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_A){ keys_down[uint8_t('a')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_S){ keys_down[uint8_t('s')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_D){ keys_down[uint8_t('d')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_Q){ keys_down[uint8_t('q')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_E){ keys_down[uint8_t('e')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  if (key == GLFW_KEY_SPACE){ keys_down[uint8_t(' ')] = (action & (GLFW_PRESS | GLFW_REPEAT)); }
  // clang-format on
};

void Canvas::mouse_scroll_callback(GLFWwindow* window,
                                   double xoffset,
                                   double yoffset) {
  camera.zoom(1.0 + 0.10 * yoffset);
}

void Canvas::mouse_motion_callback(GLFWwindow* window,
                                   double xpos,
                                   double ypos) {
  if (lmb_down && !mmb_down && !rmb_down) {
    float altitude = (ypos - mouse_y) * 0.01f;
    float azimuth = (xpos - mouse_x) * 0.01f;

    if (ImGui::GetIO().WantCaptureMouse) {
      // if the mouse is interacting with ImGui
    } else {
      camera.rotate(altitude, -azimuth);
    }

    mouse_x = xpos;
    mouse_y = ypos;
  }

  if (!lmb_down && !mmb_down && rmb_down) {
    // right click
  }
}

void Canvas::mouse_button_callback(GLFWwindow* window,
                                   int button,
                                   int action,
                                   int mods) {
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    lmb_down = true;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
  }

  if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
    rmb_down = true;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
  }

  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    lmb_down = false;
  }
  if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
    rmb_down = false;
  }
}

void Canvas::update_camera_position() {
  // clang-format off
  float scale = 1.0f;
  if (keys_down[uint8_t(' ')]) { scale = 0.1f; }
  if (keys_down[uint8_t('w')]) { camera.move_forward(scale * camera_speed); }
  if (keys_down[uint8_t('s')]) { camera.move_forward(-scale * camera_speed); }
  if (keys_down[uint8_t('a')]) { camera.move_left(scale * camera_speed); }
  if (keys_down[uint8_t('d')]) { camera.move_right(scale * camera_speed); }
  if (keys_down[uint8_t('q')]) { camera.move_down(scale * camera_speed); }
  if (keys_down[uint8_t('e')]) { camera.move_up(scale * camera_speed); }
  // clang-format on
}

void Canvas::generate_spheres() {

  static std::default_random_engine generator;
  static std::uniform_real_distribution<float> distribution(0.0, 1.0);

  spheres.resize(n);
  float w = 10.0f;

  for (int i = 0; i < n; i++) {
    spheres[i][0] = w * (2.0f * distribution(generator) - 1);
    spheres[i][1] = w * (2.0f * distribution(generator) - 1);
    spheres[i][2] = w * (2.0f * distribution(generator) - 1);
    spheres[i][3] = radius;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, spheres.size() * sizeof(glm::vec4), spheres.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

}

Canvas::Canvas() : Application(),
      vertexShader(Shader::fromString(vert_shader, GL_VERTEX_SHADER)),
      geometryShader(Shader::fromString(geom_shader, GL_GEOMETRY_SHADER)),
      fragmentShader(Shader::fromString(frag_shader, GL_FRAGMENT_SHADER)),
      shaderProgram({vertexShader, geometryShader, fragmentShader}),
      keys_down{} {
  glCheckError(__FILE__, __LINE__);

  n = 50;
  radius = 0.1;
  fov = 1.0;

  camera.lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0));
  camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

  camera_speed = 0.02;

  // vbo
  glGenBuffers(1, &vbo);
  //glBindBuffer(GL_ARRAY_BUFFER, vbo);
  //glBufferData(GL_ARRAY_BUFFER, spheres.size() * sizeof(glm::vec2), spheres.data(), GL_STATIC_DRAW);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);

  // vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // bind vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // map vbo to shader attributes
  shaderProgram.setAttribute("sphere", 4, sizeof(glm::vec4), 0);

  // vao end
  glBindVertexArray(0);

  generate_spheres();

  glfwSetWindowUserPointer(window, (void*)this);
  glfwSetKeyCallback(window, key_callback_helper);
  glfwSetScrollCallback(window, mouse_scroll_callback_helper);
  glfwSetCursorPosCallback(window, mouse_motion_callback_helper);
  glfwSetMouseButtonCallback(window, mouse_button_callback_helper);

}

void Canvas::loop() {

  // exit on window close button pressed
  if (glfwWindowShouldClose(getWindow()))
    exit();

  update_camera_position();

  // clear
  glClearColor(0.169f, 0.314f, 0.475f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // feed inputs to dear imgui, start new frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  shaderProgram.use();

  glCheckError(__FILE__, __LINE__);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glCheckError(__FILE__, __LINE__);

  camera.set_aspect(getWindowRatio());
  auto proj = camera.matrix();
  auto v = camera.up();
  auto h = glm::normalize(glm::cross(v, camera.m_pos - camera.m_focus));
  glUniformMatrix4fv(shaderProgram.uniform("proj"), 1, GL_FALSE, glm::value_ptr(proj));
  glUniform3fv(shaderProgram.uniform("camera_up"), 1, glm::value_ptr(camera.up()));
  glUniform3fv(shaderProgram.uniform("camera_position"), 1, glm::value_ptr(camera.pos()));

  glDrawArrays(GL_POINTS, 0, n);

  glBindVertexArray(0);

  shaderProgram.unuse();

  // render your GUI
  ImGui::Begin("Demo window");

  if (ImGui::DragInt("n", &n, 0.1f, 1, 1000) || 
      ImGui::DragFloat("radius", &radius, 0.01f, 0.01f, 0.5f)) {
    generate_spheres();
  }

  if (ImGui::DragFloat("fov", &fov, 0.01f, 0.05f, 1.5f)) {
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);
  }

  ImGui::End();

  // Render dear imgui into screen
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

int main(int argc, const char* argv[]) {
  Canvas app;
  app.run();
  return 0;
}
