cmake_minimum_required (VERSION 3.8)

project ("XScript")

add_executable (XScript "main.cpp" "src/grammar.h" "src/grammar.cpp" "src/meta.h" "src/meta.cpp" "src/ast.h" "src/ast.cpp" "src/runtime.h" "src/runtime.cpp" "src/context.h" "src/context.cpp" "src/type.h" "src/type.cpp" "src/compiler.h" "src/compiler.cpp" "src/pass.h" "src/pass.cpp" "src/object.h" "src/object.cpp" "src/value.h" "src/value.cpp" "src/xstdlib.h" "src/xstdlib.cpp" "src/symbols.h" "src/symbols.cpp" "src/library.h" "src/library.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XScript PROPERTY CXX_STANDARD 20)
endif()
