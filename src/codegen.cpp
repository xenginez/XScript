#include "codegen.h"

#include "module.h"
#include "symbols.h"

x::module_scanner_visitor::module_scanner_visitor( const x::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}

void x::module_scanner_visitor::visit( x::unit_ast * val )
{
	_module->name = val->location.file;

	visitor::visit( val );
}

void x::module_scanner_visitor::visit( x::enum_decl_ast * val )
{
	x::type_section::item item;

	item.flag = x::type_section::flag_t::ENUM;
	item.size = sizeof( x::int64 );
	item.name = _module->transfer( symbols()->find_symbol_from_ast( val->shared_from_this() )->fullname );

	_module->types.items.push_back( item );
}

void x::module_scanner_visitor::visit( x::class_decl_ast * val )
{
	x::type_section::item item;

	item.flag = x::type_section::flag_t::CLASS;
	item.size = sizeof( x::int64 );
	item.name = _module->transfer( symbols()->find_symbol_from_ast( val->shared_from_this() )->fullname );

	_module->types.items.push_back( item );
}

void x::module_scanner_visitor::visit( x::template_decl_ast * val )
{
}

x::module_generater_visitor::module_generater_visitor( const x::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}

x::llvm_scanner_visitor::llvm_scanner_visitor( const llvm::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}

x::llvm_generater_visitor::llvm_generater_visitor( const llvm::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}

x::spirv_scanner_visitor::spirv_scanner_visitor( const spirv::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}

x::spirv_generater_visitor::spirv_generater_visitor( const spirv::module_ptr & module, const x::symbols_ptr & symbols )
	: scope_with_visitor( symbols ), _module( module )
{
}
