cmake_minimum_required (VERSION 3.8)

project ("XScript")

add_executable (XScript 
"main.cpp"
"src/type.h"
"src/flags.hpp"
"src/ast.h"
"src/ast.cpp"
"src/meta.h"
"src/meta.cpp"
"src/pass.h"
"src/pass.cpp"
"src/module.h"
"src/module.cpp"
"src/float16.h"
"src/float16.cpp"
"src/grammar.h"
"src/grammar.cpp"
"src/symbols.h"
"src/symbols.cpp"
"src/context.h"
"src/context.cpp"
"src/compiler.h"
"src/compiler.cpp"
"src/exception.h"
"src/exception.cpp"
"src/value.h"
"src/value.cpp"
"src/object.h"
"src/object.cpp" "src/runtime.h" "src/runtime.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XScript PROPERTY CXX_STANDARD 20)
endif()
