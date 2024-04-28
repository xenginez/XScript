cmake_minimum_required (VERSION 3.8)

project ("XScript")

add_executable (XScript 
"main.cpp"
"src/type.h"
"src/ast.h"
"src/ast.cpp"
"src/meta.h"
"src/meta.cpp"
"src/pass.h"
"src/pass.cpp"
"src/float16.h"
"src/float16.cpp"
"src/grammar.h"
"src/grammar.cpp"
"src/section.h"
"src/section.cpp"
"src/symbols.h"
"src/symbols.cpp"
"src/library.h"
"src/library.cpp"
"src/context.h"
"src/context.cpp"
"src/compiler.h"
"src/compiler.cpp" "src/symbol.h" "src/symbol.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XScript PROPERTY CXX_STANDARD 20)
endif()
