#include "semantic.h"

#include "symbols.h"
#include "exception.h"

x::symbol_scanner_visitor::symbol_scanner_visitor( const x::symbols_ptr & val )
	: scope_with_visitor( val )
{
}

void x::symbol_scanner_visitor::visit( x::unit_ast * val )
{
	symbols()->add_unit( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->add_enum( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::using_decl_ast * val )
{
	symbols()->add_alias( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::class_decl_ast * val )
{
	symbols()->add_class( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::element_decl_ast * val )
{
	symbols()->add_element( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::variable_decl_ast * val )
{
	auto sym = symbols()->add_variable( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::function_decl_ast * val )
{
	symbols()->add_function( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::parameter_decl_ast * val )
{
	symbols()->add_param( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::template_decl_ast * val )
{
	symbols()->add_template( val );
	
	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::namespace_decl_ast * val )
{
	symbols()->add_namespace( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::compound_stat_ast * val )
{
	symbols()->add_block( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::while_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::for_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::foreach_stat_ast * val )
{
	symbols()->add_cycle( val );

	scope_with_visitor::visit( val );
}

void x::symbol_scanner_visitor::visit( x::local_stat_ast * val )
{
	symbols()->add_local( val );

	scope_with_visitor::visit( val );
}
