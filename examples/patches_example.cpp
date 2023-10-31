#include <array>
#include <vector>
#include <random>
#include <iostream>

#include "glError.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Application.hpp"

#include "patches.hpp"

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

class PatchDemo : public Application {
 public:
  PatchDemo() : Application("PatchDemo") {
    fov = 1.0;
    camera_speed = 0.02;
    camera.lookAt(glm::vec3(4, 4, 4), glm::vec3(0, 0, 0));
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

    subdivisions = 4;
    remesh();
  }

  void remesh() {
    patches.clear();
    patches.set_subdivision(PatchType::QUAD4, subdivisions);
    patches.set_color(yellow);
    patches.append(Quad4{{{0, 0, 0.7}, {1, 0, 0}, {1, 1, 0.7}, {0, 1, 0}}});

    patches.set_subdivision(PatchType::TRI6, subdivisions);
    patches.set_color(red);
    patches.append(Tri6{{{2, 0, 0.5}, {3, 0, 0}, {2, 1, 0}, 
                         {2.5, 0.0, 0.0}, {2.5, 0.5, 0}, {2.0, 0.5, 0.0}}});

    patches.set_subdivision(PatchType::QUAD8, subdivisions);
    patches.set_color(green);
    patches.append(Quad8{{{2.0, 2.0, 0.5}, {3.0, 2.0, 0.5}, {3.0, 3.0, 0.5}, {2.0, 3.0, 0.5},
                          {2.5, 2.0, 0.0}, {3.0, 2.5, 1.0}, {2.5, 3.0, 0.0}, {2.0, 2.5, 1.0}}});

    patches.set_subdivision(PatchType::QUAD9, subdivisions);
    patches.set_color(purple);
    patches.append(Quad9{{{0.0, 2.0, 0.0}, {1.0, 2.0, 0.0}, {1.0, 3.0, 0.0}, {0.0, 3.0, 0.0},
                          {0.5, 2.0, 0.0}, {1.0, 2.5, 0.0}, {0.5, 3.0, 0.0}, {0.0, 2.5, 0.0}, {0.5, 2.5, 0.5}}});

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

    patches.draw(camera);

    // render your GUI
    ImGui::Begin("Demo window");

    if (ImGui::DragFloat("fov", &fov, 0.01f, 0.05f, 1.5f)) {
      camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);
    }

    if (ImGui::DragInt("subdivision", &subdivisions, 0.05f, 1, 16)) {
      remesh();
    }

    static float light_intensity = 0.0f;
    if (ImGui::DragFloat("light intensity", &light_intensity, 0.01f, 0.0f, 1.0f)) {
      glm::vec3 direction(0.721995, 0.618853, 0.309426);
      patches.set_light(direction, light_intensity);
    }

    static int which = 0;

    ImGui::End();

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  }

 private:
  float fov;
  Patches patches;
  int subdivisions;
};

int main(int argc, const char* argv[]) {
  PatchDemo app;
  app.run();
  return 0;
}
