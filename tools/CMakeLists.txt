add_executable(xscompiler "xscompiler.cpp")
target_include_directories(xscompiler PRIVATE "../src")
target_link_libraries(xscompiler PRIVATE xs)
set_property(TARGET xscompiler PROPERTY CXX_STANDARD 20)
