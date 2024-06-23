cmake_minimum_required (VERSION 3.8)

project ("XScript")

add_executable (XScript 
"main.cpp"
"src/flags.hpp"
"src/endian.hpp"
"src/float16.h"
"src/float16.cpp"
"src/type.h"
"src/ast.h"
"src/ast.cpp"
"src/meta.h"
"src/meta.cpp"
"src/module.h"
"src/module.cpp"
"src/grammar.h"
"src/grammar.cpp"
"src/builtin.h"
"src/builtin.cpp"
"src/visitor.h"
"src/visitor.cpp"
"src/symbols.h"
"src/symbols.cpp"
"src/codegen.h"
"src/codegen.cpp"
"src/semantic.h"
"src/semantic.cpp"
"src/compiler.h"
"src/compiler.cpp"
"src/exception.h"
"src/exception.cpp"
"src/value.h"
"src/value.cpp"
"src/object.h"
"src/object.cpp"
"src/runtime.h"
"src/runtime.cpp"
"src/context.h"
"src/context.cpp"
"src/allocator.h"
"src/allocator.cpp"
"src/interpreter.h"
"src/interpreter.cpp"
"src/virtual_machine.h"
"src/virtual_machine.cpp"

"src/lib/x_lib.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XScript PROPERTY CXX_STANDARD 20)
endif()
