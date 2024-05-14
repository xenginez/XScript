#include "pass.h"

#include "symbols.h"

x::pass::pass( const x::symbols_ptr & val )
	: _symbols( val )
{
}

const x::symbols_ptr & x::pass::symbols() const
{
	return _symbols;
}

x::pass_with_scope::pass_with_scope( const x::symbols_ptr & val )
	: pass( val )
{
}

x::symbol_scanner_pass::symbol_scanner_pass( const x::symbols_ptr & val )
	: pass_with_scope( val )
{
}

x::reference_solver_pass::reference_solver_pass( const x::symbols_ptr & val )
	: pass_with_scope( val )
{
}

x::type_checker_pass::type_checker_pass( const x::symbols_ptr & val )
	: pass( val )
{
}

x::semantic_checker_pass::semantic_checker_pass( const x::symbols_ptr & val )
	: pass( val )
{
}

void x::pass_with_scope::visit( x::unit_ast * val )
{
	symbols()->push_scope( val->location.file );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope(val->name );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::template_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( std::format( "block_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::pass_with_scope::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		pass::visit( val );
	}
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::unit_ast * val )
{
	symbols()->add_unit( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::enum_decl_ast * val )
{
	symbols()->add_enum( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::using_decl_ast * val )
{
	symbols()->add_alias( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::class_decl_ast * val )
{
	symbols()->add_class( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::element_decl_ast * val )
{
	symbols()->add_element( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::variable_decl_ast * val )
{
	auto sym = symbols()->add_variable( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::function_decl_ast * val )
{
	symbols()->add_function( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::parameter_decl_ast * val )
{
	symbols()->add_param( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::template_decl_ast * val )
{
	symbols()->add_template( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::namespace_decl_ast * val )
{
	symbols()->add_namespace( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::compound_stat_ast * val )
{
	symbols()->add_block( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::while_stat_ast * val )
{
	symbols()->add_cycle( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::for_stat_ast * val )
{
	symbols()->add_cycle( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::foreach_stat_ast * val )
{
	symbols()->add_cycle( val );

	pass_with_scope::visit( val );
}

void x::symbol_scanner_pass::visit( x::local_stat_ast * val )
{
	symbols()->add_local( val );

	pass_with_scope::visit( val );
}
