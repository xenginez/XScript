add_executable(xcompiler "xcompiler.cpp")
target_include_directories(xcompiler PRIVATE "../src")
target_link_libraries(xcompiler PRIVATE xs)
set_property(TARGET xcompiler PROPERTY CXX_STANDARD 20)

add_executable(xlserver "xlserver.cpp")
target_include_directories(xlserver PRIVATE "../src")
target_link_libraries(xlserver PRIVATE xs)
set_property(TARGET xlserver PROPERTY CXX_STANDARD 20)
