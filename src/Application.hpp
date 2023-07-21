/**
 * Application.cpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#ifndef OPENGL_CMAKE_SKELETON_APPLICATION_HPP
#define OPENGL_CMAKE_SKELETON_APPLICATION_HPP

#include <string>

#include "Camera.hpp"

struct GLFWwindow;

/// Application class:
/// * init OpenGL
/// * provide:
///   * getWidth()
///   * getHeight()
///   * getFrameDeltaTime()
///   * getWindowRatio()
///   * windowDimensionChanged()
/// * let the user define the "loop" function.
class Application {
 public:
  Application(std::string title = "Application");

  virtual void update_camera_position();
  virtual void mouse_motion_callback(GLFWwindow* window, double xpos, double ypos);
  virtual void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  virtual void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  virtual void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

  static Application& getInstance();

  // get the window id
  GLFWwindow* getWindow() const;

  // window control
  void exit();

  // delta time between frame and time from beginning
  float getFrameDeltaTime() const;
  float getTime() const;

  // application run
  void run();

  // Application informations
  //
  int getWidth();
  int getHeight();
  float getWindowRatio();
  bool windowDimensionChanged();

 protected:
  enum State { stateReady, stateRun, stateExit };

  State state;

  Application& operator=(const Application&) { return *this; }

  GLFWwindow* window;

  // Time:
  float time;
  float deltaTime;

  // Dimensions:
  int width;
  int height;
  bool dimensionChanged;
  void detectWindowDimensionChange();

  Camera camera;

  float camera_speed;
  bool keys_down[256];
  double mouse_x, mouse_y;

  bool lmb_down = false;
  bool mmb_down = false;
  bool rmb_down = false;

  Application(const Application&) {};

  virtual void loop();
};

#endif /* end of include guard: OPENGL_CMAKE_SKELETON_APPLICATION_HPP */
