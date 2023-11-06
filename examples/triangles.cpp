#include <array>
#include <vector>
#include <random>
#include <iostream>

#include "glError.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Application.hpp"

#include "triangles.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "misc/json.hpp"

#include <fstream>
#include <unordered_map>

using namespace colors;
using namespace Graphics;

class TriangleDemo : public Application {
 public:
  TriangleDemo() : Application("TriangleDemo") {
    fov = 1.0;
    camera_speed = 0.02;
    camera.lookAt(glm::vec3(4, 4, 4), glm::vec3(0, 0, 0));
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

    tris.set_color(yellow);
    tris.append(Tri3{{{0, 0, 0}, {1, 0, 0}, {1, 1, 0.4}}});
    //tris.set_color(purple);
    //tris.append(Tri6{{{1, 0, -0.3}, {1, 1, 0.3}, {0, 1, 0}, {1.0, 0.5, 0}, {0.5, 1, 0}, {0.5, 0.5, 0.0}}});
  }

 protected:
  virtual void loop() {

    // exit on window close button pressed
    if (glfwWindowShouldClose(getWindow()))
      exit();

    update_camera_position();

    // clear
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    tris.draw(camera);

    // render your GUI
    ImGui::Begin("Demo window");

    if (ImGui::DragFloat("fov", &fov, 0.01f, 0.05f, 1.5f)) {
      camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);
    }

    static float light_intensity = 0.0f;
    if (ImGui::DragFloat("light intensity", &light_intensity, 0.01f, 0.0f, 1.0f)) {
      glm::vec3 direction(0.721995, 0.618853, 0.309426);
      tris.set_light(direction, light_intensity);
    }

    static int which = 0;

    ImGui::End();

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  }

 private:
  float fov;
  Triangles tris;
};

int main(int argc, const char* argv[]) {
  TriangleDemo app;
  app.run();
  return 0;
}
