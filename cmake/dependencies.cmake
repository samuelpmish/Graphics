include(FetchContent)

add_definitions(-DGLEW_STATIC)

FetchContent_Declare(
  TPL_glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
message("resolving dependencies: glm")
FetchContent_MakeAvailable(TPL_glm)

FetchContent_Declare(
  TPL_GLEW
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
  GIT_TAG glew-cmake-2.2.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
message("resolving dependencies: glew")
FetchContent_MakeAvailable(TPL_GLEW)

FetchContent_Declare(
  TPL_glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.3.8
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_USE_CHDIR OFF CACHE BOOL " " FORCE)
message("resolving dependencies: glfw")
FetchContent_MakeAvailable(TPL_glfw)

add_library(imgui STATIC 
  ${PROJECT_SOURCE_DIR}/src/imgui/imconfig.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_demo.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_internal.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/imstb_rectpack.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imstb_textedit.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imstb_truetype.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl3.h
  ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl3.cpp

  ${PROJECT_SOURCE_DIR}/src/imgui/implot.h
  ${PROJECT_SOURCE_DIR}/src/imgui/implot.cpp
  ${PROJECT_SOURCE_DIR}/src/imgui/implot_internal.h
  ${PROJECT_SOURCE_DIR}/src/imgui/implot_items.cpp
)

target_include_directories(imgui PUBLIC .)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui PUBLIC glfw libglew_static)