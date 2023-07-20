file(GLOB cpp_examples ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(filename ${cpp_examples})		

  get_filename_component(testname ${filename} NAME_WE)

  add_executable(${testname} ${filename})
  target_link_libraries(${testname} PUBLIC graphics)
  target_compile_definitions(${testname} PUBLIC "-DDATA_DIR=\"${PROJECT_SOURCE_DIR}/data/\"")

endforeach(filename ${cpp_examples})
