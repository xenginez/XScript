add_executable(parse_test "parse_test.cpp")
target_include_directories(parse_test PRIVATE "../src")
target_link_libraries(parse_test PRIVATE xs)
set_property(TARGET parse_test PROPERTY CXX_STANDARD 20)
