cmake_minimum_required (VERSION 3.15)
project (GraphicsProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include(cmake/dependencies.cmake)

add_library(graphics STATIC
  src/Camera.hpp
  src/Camera.cpp
  src/Application.hpp
  src/Application.cpp
  src/glError.hpp
  src/glError.cpp
  src/Shader.hpp
  src/Shader.cpp
  #src/Scene.hpp
  #src/Scene.cpp
  src/spheres.hpp
  src/spheres.cpp
  src/cylinders.hpp
  src/cylinders.cpp
  src/triangles.hpp
  src/triangles.cpp
  src/patches.hpp
  src/patches.cpp
  src/patch_shader_tri6.cpp
  src/patch_shader_quad4.cpp
  src/patch_shader_quad8.cpp
  src/patch_shader_quad9.cpp
)

target_include_directories(graphics PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(graphics PUBLIC ${PROJECT_SOURCE_DIR}/src/imgui)
target_link_libraries(graphics PUBLIC glfw glm libglew_static imgui)
target_compile_definitions(graphics PUBLIC "-DGRAPHICS_DATA_DIR=\"${PROJECT_SOURCE_DIR}/data/\"")

if (GRAPHICS_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (GRAPHICS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if (GRAPHICS_ENABLE_ASAN)
  add_link_options(-fsanitize=address)
  add_compile_options(-fsanitize=address)
endif()