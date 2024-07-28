add_executable(parse_test "parse_test.cpp")
target_include_directories(parse_test PRIVATE "../src")
target_link_libraries(parse_test PRIVATE xs)
set_property(TARGET parse_test PROPERTY CXX_STANDARD 20)

add_executable(json_test "json_test.cpp")
target_include_directories(json_test PRIVATE "../src")
target_link_libraries(json_test PRIVATE xs)
set_property(TARGET json_test PROPERTY CXX_STANDARD 20)

add_executable(zip_test "zip_test.cpp")
target_include_directories(zip_test PRIVATE "../src")
target_link_libraries(zip_test PRIVATE xs)
set_property(TARGET zip_test PROPERTY CXX_STANDARD 20)
