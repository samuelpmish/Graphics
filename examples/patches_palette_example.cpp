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

std::vector< glm::vec3 > vertices = {
    {0., 0., -1.}, {0., 0., 1.}, {-0.894427, 0., -0.447214}, {0.894427, 0., 0.447214}, {0.723607, -0.525731, -0.447214}, {0.723607, 0.525731, -0.447214}, {-0.723607, -0.525731, 0.447214}, {-0.723607, 0.525731, 0.447214}, {-0.276393, -0.850651, -0.447214}, {-0.276393, 0.850651, -0.447214}, {0.276393, -0.850651, 0.447214}, {0.276393, 0.850651, 0.447214}, {-0.425325, 0.309017, 0.850651}, {0.16246, 0.5, 0.850651}, {-0.262866, 0.809017, 0.525731}, {-0.425325, -0.309017, 0.850651}, {-0.850651, 0., 0.525731}, {0.16246, -0.5, 0.850651}, {-0.262866, -0.809017, 0.525731}, {0.525731, 0., 0.850651}, {0.688191, -0.5, 0.525731}, {0.688191, 0.5, 0.525731}, {0.425325, -0.309017, -0.850651}, {0.262866, -0.809017, -0.525731}, {-0.16246, -0.5, -0.850651}, {0.425325, 0.309017, -0.850651}, {0.850651, 0., -0.525731}, {-0.16246, 0.5, -0.850651}, {0.262866, 0.809017, -0.525731}, {-0.525731, 0., -0.850651}, {-0.688191, 0.5, -0.525731}, {-0.688191, -0.5, -0.525731}, {0., 1., 0.}, {-0.587785, 0.809017, 0.}, {-0.951057, 0.309017, 0.}, {-0.951057, -0.309017, 0.}, {-0.587785, -0.809017, 0.}, {0., -1., 0.}, {0.587785, -0.809017, 0.}, {0.951057, -0.309017, 0.}, {0.951057, 0.309017, 0.}, {0.587785, 0.809017, 0.}};

std::vector<std::array<int, 3>> tris = {
    {1, 13, 12}, {12, 13, 14}, {12, 14, 7}, {13, 11, 14}, {1, 12, 15}, {15, 12, 16}, {15, 16, 6}, {12, 7, 16}, {1, 15, 17}, {17, 15, 18}, {17, 18, 10}, {15, 6, 18}, {1, 17, 19}, {19, 17, 20}, {19, 20, 3}, {17, 10, 20}, {1, 19, 13}, {13, 19, 21}, {13, 21, 11}, {19, 3, 21}, {4, 23, 22}, {22, 23, 24}, {22, 24, 0}, {23, 8, 24}, {5, 26, 25}, {25, 26, 22}, {25, 22, 0}, {26, 4, 22}, {9, 28, 27}, {27, 28, 25}, {27, 25, 0}, {28, 5, 25}, {2, 30, 29}, {29, 30, 27}, {29, 27, 0}, {30, 9, 27}, {8, 31, 24}, {24, 31, 29}, {24, 29, 0}, {31, 2, 29}, {11, 32, 14}, {14, 32, 33}, {14, 33, 7}, {32, 9, 33}, {7, 34, 16}, {16, 34, 35}, {16, 35, 6}, {34, 2, 35}, {6, 36, 18}, {18, 36, 37}, {18, 37, 10}, {36, 8, 37}, {10, 38, 20}, {20, 38, 39}, {20, 39, 3}, {38, 4, 39}, {3, 40, 21}, {21, 40, 41}, {21, 41, 11}, {40, 5, 41}, {4, 38, 23}, {23, 38, 37}, {23, 37, 8}, {38, 10, 37}, {5, 40, 26}, {26, 40, 39}, {26, 39, 4}, {40, 3, 39}, {9, 32, 28}, {28, 32, 41}, {28, 41, 5}, {32, 11, 41}, {2, 34, 30}, {30, 34, 33}, {30, 33, 9}, {34, 7, 33}, {8, 36, 31}, {31, 36, 35}, {31, 35, 2}, {36, 6, 35}};

class PaletteDemo : public Application {
 public:
  PaletteDemo() : Application("PaletteDemo") {
    fov = 1.0;
    camera_speed = 0.02;
    camera.lookAt(glm::vec3(5, 5, 4), glm::vec3(1, 1, 0));
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

    patches.set_value_bounds(-1.0, 1.0);

    subdivisions = 4;
    remesh();

  }

  void remesh() {
    patches.clear();
    patches.set_subdivision(PatchType::TRI6, subdivisions);



    auto normalized_avg = [](glm::vec3 a, glm::vec3 b) {
      return normalize(a+b);
    };

    auto xyzz = [](glm::vec3 x) { return glm::vec4{x[0], x[1], x[2], x[2]}; };

    for (auto [i, j, k] : tris) {
      glm::vec3 v[3] = {vertices[i], vertices[j], vertices[k]};
      patches.append(Tri6v{{
        xyzz(v[0]), 
        xyzz(v[1]), 
        xyzz(v[2]), 
        xyzz(normalized_avg(v[0], v[1])), 
        xyzz(normalized_avg(v[1], v[2])), 
        xyzz(normalized_avg(v[2], v[0]))
      }});
    }

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

    if (ImGui::DragInt("palette", &which_palette, 0.05f, 0, 4)) {
      switch(which_palette) {
        case 0:
          patches.set_palette(palettes::warm);
          break;
        case 1:
          patches.set_palette(palettes::cold);
          break;
        case 2:
          patches.set_palette(palettes::purple_to_yellow);
          break;
        case 3:
          patches.set_palette(palettes::blue_to_red);
          break;
      }
      remesh();
    }

    if (ImGui::DragInt("posterization", &posterize, 0.05f, 0, 16)) {
      patches.posterization(posterize);
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
  int which_palette;
  int posterize;
};

int main(int argc, const char* argv[]) {
  PaletteDemo app;
  app.run();
  return 0;
}
