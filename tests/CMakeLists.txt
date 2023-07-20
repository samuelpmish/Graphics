file(GLOB cpp_tests ${PROJECT_SOURCE_DIR}/tests/*.cpp)

foreach(filename ${cpp_tests})		

  get_filename_component(testname ${filename} NAME_WE)

  add_executable(${testname} ${filename})
  target_link_libraries(${testname} PUBLIC graphics)
  target_compile_definitions(${testname} PUBLIC "-DDATA_DIR=\"${PROJECT_SOURCE_DIR}/data/\"")

endforeach(filename ${cpp_tests})
