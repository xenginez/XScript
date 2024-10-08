#include "codegen.h"

#include "module.h"
#include "symbols.h"

void x::module_generater::generate( const x::module_ptr & module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast )
{
}

void x::llvm_module_generater::generate( const llvm::module_ptr & llvm_module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::module_ptr & module )
{
}

void x::spirv_module_generater::generate( const spirv::module_ptr & spirv_module, const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::module_ptr & module )
{
}
