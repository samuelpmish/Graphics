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
  Molecules() : Application("Molecules") {
    m = Molecule::import_from_json(DATA_DIR"citric_acid.json"); 

    fov = 1.0;
    camera_speed = 0.02;
    camera.lookAt(glm::vec3(4, 4, 4), glm::vec3(0, 0, 0));
    camera.perspective(fov, getWindowRatio(), 0.01f, 100.0f);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
      m = Molecule::import_from_json(DATA_DIR"citric_acid.json"); 
    }
    if (ImGui::RadioButton("guanine", &which, 1)) {
      m = Molecule::import_from_json(DATA_DIR"guanine.json"); 
    }
    if (ImGui::RadioButton("CUVNAK", &which, 2)) {
      m = Molecule::import_from_json(DATA_DIR"CUVNAK.json"); 
    }

    ImGui::End();

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  }

 private:
  float fov;

  Molecule m;
};

int main(int argc, const char* argv[]) {
  Molecules app;
  app.run();
  return 0;
}
