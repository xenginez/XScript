cmake_minimum_required (VERSION 3.8)

project ("XScript")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)

configure_file(scripts/test.xs ${CMAKE_CURRENT_BINARY_DIR}/test.xs COPYONLY)
configure_file(scripts/script_0.xs ${CMAKE_CURRENT_BINARY_DIR}/script_0.xs COPYONLY)