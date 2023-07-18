file(GLOB cpp_examples ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(filename ${cpp_examples})		

  get_filename_component(testname ${filename} NAME_WE)

  add_executable(${testname} ${filename})
  target_link_libraries(${testname} PUBLIC graphics)

endforeach(filename ${cpp_examples})
