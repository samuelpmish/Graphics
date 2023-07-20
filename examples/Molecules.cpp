#include <array>
#include <vector>
#include <random>
#include <iostream>

#include "glError.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Application.hpp"

#include "spheres.hpp"
#include "cylinders.hpp"

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

static const std::unordered_map< uint32_t, rgbcolor > color_from_atomic_number = {
  { 1, rgbcolor{230, 230, 230, 255}}, // hydrogen is white
  { 6, rgbcolor{170, 170, 170, 255}}, // carbon is light gray
  { 7, rgbcolor{ 30,  30, 200, 255}}, // nitrogen is blue
  { 8, rgbcolor{230,  30,  30, 255}}, // oxygen is red
  {15, rgbcolor{240, 140,   0, 255}}, // phosphorus is orange
  {16, rgbcolor{240, 220,   0, 255}}, // sulfur is yellow
  {30, rgbcolor{170,  20, 170, 255}}, // zinc is purple
};

struct Molecule {

  static Molecule import_from_json(std::string filename) {

    float atom_radius = 0.3; 
    float bond_radius = 0.1; 

    // read the contents of the entire file into str
    std::ifstream infile(filename);
    std::string str;
    if (infile) {
      infile.seekg(0, std::ios::end);   
      str.reserve(infile.tellg());
      infile.seekg(0, std::ios::beg);
      str.assign((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    } else {
      std::cout << "file: " << filename << " not found. exiting ... " << std::endl;
      exit(1);
    }

    auto j = nlohmann::json::parse(str);

    glm::vec3 offset{};

    std::vector < Sphere > atoms;
    std::vector < rgbcolor > atom_colors;
    for (auto & atom : j["atoms"]) {
      atoms.push_back(Sphere{
        {atom["coordinates"][0], atom["coordinates"][1], atom["coordinates"][2]}, 
        atom_radius
      });
      atom_colors.push_back(color_from_atomic_number.at(atom["atomic_number"].get<uint32_t>()));

      offset += atoms.back().center;
    }

    offset /= atoms.size();
    for (auto & atom : atoms) { atom.center -= offset; }

    std::vector < Cylinder > bonds;
    std::vector < rgbcolor > bond_colors;
    for (auto & bond : j["bonds"]) {
      Sphere start = atoms[bond[0]];
      Sphere mid = {(atoms[bond[0]].center + atoms[bond[1]].center) * 0.5f, bond_radius};
      Sphere end = atoms[bond[1]];

      start.radius = mid.radius = end.radius = bond_radius * ((bond[2] == 2) ? 1.7 : 1.0);

      bonds.push_back(Cylinder{start, mid});
      bond_colors.push_back(atom_colors[bond[0]]);

      bonds.push_back(Cylinder{mid, end});
      bond_colors.push_back(atom_colors[bond[1]]);
   }

    Molecule m;
    m.spheres.append(atoms, atom_colors);
    m.cylinders.append(bonds, bond_colors);
    return m;
  }

  void draw(const Camera & camera) {
    spheres.draw(camera);
    cylinders.draw(camera);
  }

  Spheres spheres;
  Cylinders cylinders;
};

class Molecules : public Application {
 public:
  Molecules();

  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_motion_callback(GLFWwindow* window, double xpos, double ypos);
  void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  void update_camera_position();

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

  float fov;

  Molecule m;
};

#if defined(__APPLE__)
float DPI_scale = 2;
#else
float DPI_scale = 1;
#endif

// clang-format off
void key_callback_helper(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto mesher = (Molecules*)glfwGetWindowUserPointer(window);
  mesher->key_callback(window, key, scancode, action, mods);
}

void mouse_scroll_callback_helper(GLFWwindow* window, double xoffset, double yoffset) {
  auto mesher = (Molecules*)glfwGetWindowUserPointer(window);
  mesher->mouse_scroll_callback(window, xoffset, yoffset);
}

void mouse_motion_callback_helper(GLFWwindow* window, double xpos, double ypos) {
  auto mesher = (Molecules*)glfwGetWindowUserPointer(window);
  mesher->mouse_motion_callback(window, xpos, ypos);
}

void mouse_button_callback_helper(GLFWwindow* window, int button, int action, int mods) {
  auto mesher = (Molecules*)glfwGetWindowUserPointer(window);
  mesher->mouse_button_callback(window, button, action, mods);
}
// clang-format on

void Molecules::key_callback(GLFWwindow* window,
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

void Molecules::mouse_scroll_callback(GLFWwindow* window,
                                   double xoffset,
                                   double yoffset) {
  camera.zoom(1.0 + 0.10 * yoffset);
}

void Molecules::mouse_motion_callback(GLFWwindow* window,
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

void Molecules::mouse_button_callback(GLFWwindow* window,
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

void Molecules::update_camera_position() {
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

Molecules::Molecules() : Application(), keys_down{} {

  m = Molecule::import_from_json("/Users/sam/code/Graphics/data/citric_acid.json"); 

  glCheckError(__FILE__, __LINE__);

  fov = 1.0;

  camera.lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0));
  camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

  camera_speed = 0.02;

  glfwSetWindowUserPointer(window, (void*)this);
  glfwSetKeyCallback(window, key_callback_helper);
  glfwSetScrollCallback(window, mouse_scroll_callback_helper);
  glfwSetCursorPosCallback(window, mouse_motion_callback_helper);
  glfwSetMouseButtonCallback(window, mouse_button_callback_helper);

}

void Molecules::loop() {

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

  m.draw(camera);

  // render your GUI
  ImGui::Begin("Demo window");

  if (ImGui::DragFloat("fov", &fov, 0.01f, 0.05f, 1.5f)) {
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);
  }

  static float light_intensity = 0.0f;
  if (ImGui::DragFloat("light intensity", &light_intensity, 0.01f, 0.0f, 1.0f)) {
    glm::vec3 direction(0.721995, 0.618853, 0.309426);
    m.spheres.set_light(direction, light_intensity);
    m.cylinders.set_light(direction, light_intensity);
  }

  static int which = 0;

  if (ImGui::RadioButton("citric acid", &which, 0)) {
    m = Molecule::import_from_json("/Users/sam/code/Graphics/data/citric_acid.json"); 
  }
  if (ImGui::RadioButton("guanine", &which, 1)) {
    m = Molecule::import_from_json("/Users/sam/code/Graphics/data/guanine.json"); 
  }
  if (ImGui::RadioButton("CUVNAK", &which, 2)) {
    m = Molecule::import_from_json("/Users/sam/code/Graphics/data/CUVNAK.json"); 
  }


  ImGui::End();

  // Render dear imgui into screen
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

int main(int argc, const char* argv[]) {
  Molecules app;
  app.run();
  return 0;
}
