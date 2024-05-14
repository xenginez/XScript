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

x::type_checker_pass::type_checker_pass( const x::symbols_ptr & val )
	: pass_with_scope( val )
{
}

x::semantic_checker_pass::semantic_checker_pass( const x::symbols_ptr & val )
	: pass_with_scope( val )
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

void x::type_checker_pass::visit( x::type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::temp_type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::func_type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::array_type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::element_decl_ast * val )
{
	ASSERT( is_const_int( val->value.get() ), "" );

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::assignment_exp_ast * val )
{
	switch ( val->token )
	{
	case x::token_t::TK_ASSIGN:               // =
	case x::token_t::TK_ADD_ASSIGN:           // +=
	case x::token_t::TK_SUB_ASSIGN:           // -= 
	case x::token_t::TK_MUL_ASSIGN:           // *=
	case x::token_t::TK_DIV_ASSIGN:           // /= 
		ASSERT( get_expr_type( val->left.get() ) == get_expr_type( val->right.get() ), "" );
		break;
	case x::token_t::TK_MOD_ASSIGN:           // %= 
	case x::token_t::TK_AND_ASSIGN:           // &= 
	case x::token_t::TK_OR_ASSIGN:            // |= 
	case x::token_t::TK_XOR_ASSIGN:           // ^= 
	case x::token_t::TK_LSHIFT_EQUAL:         // <<=
	case x::token_t::TK_RSHIFT_EQUAL:         // >>=
		ASSERT( is_const_int( val->left.get() ) == is_const_int( val->right.get() ), "" );
		break;
	}

	pass_with_scope::visit( val );
}

void x::type_checker_pass::visit( x::index_exp_ast * val )
{
	ASSERT( is_const_int( val->right.get() ), "" );

	pass_with_scope::visit( val );
}

bool x::type_checker_pass::is_const_int( x::exp_stat_ast * val ) const
{
	return false;
}

x::type_symbol * x::type_checker_pass::get_expr_type( x::exp_stat_ast * val ) const
{
	return nullptr;
}

void x::semantic_checker_pass::visit( x::break_stat_ast * val )
{
}

void x::semantic_checker_pass::visit( x::return_stat_ast * val )
{
}

void x::semantic_checker_pass::visit( x::continue_stat_ast * val )
{
}

void x::semantic_checker_pass::visit( x::assignment_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::unary_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::postfix_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::index_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::invoke_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::member_exp_ast * val )
{
}

void x::semantic_checker_pass::visit( x::identifier_exp_ast * val )
{
}
