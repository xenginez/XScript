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
	// symbols()->add_template( val );
	// 
	// pass_with_scope::visit( val );
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

x::instantiate_class_visitor::instantiate_class_visitor( const x::symbols_ptr & val )
	: scope_with_visitor( val )
{
}

void x::instantiate_class_visitor::visit( x::temp_type_ast * val )
{
	//symbols()->find_template_symbols()
}

bool x::instantiate_class_visitor::matching( x::template_decl_ast * temp, x::temp_type_ast * type ) const
{
	return false;
}

x::class_decl_ast_ptr x::instantiate_class_visitor::instantiate( x::closure_expr_ast * closure ) const
{
	return x::class_decl_ast_ptr();
}

x::class_decl_ast_ptr x::instantiate_class_visitor::instantiate( x::template_decl_ast * temp, x::temp_type_ast * type ) const
{
	return x::class_decl_ast_ptr();
}

x::semantic_checker_visitor::semantic_checker_visitor( const x::symbols_ptr & val )
	: scope_with_visitor( val )
{
}

void x::semantic_checker_visitor::visit( x::type_ast * val )
{
	XTHROW( x::semantic_exception, symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::temp_type_ast * val )
{
	//XTHROW( x::semantic_exception, symbols()->find_type_symbol( val->name ) != nullptr, "" );
	//
	//pass_with_scope::visit( val );
}

void x::semantic_checker_visitor::visit( x::func_type_ast * val )
{
	XTHROW( x::semantic_exception, symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::array_type_ast * val )
{
	XTHROW( x::semantic_exception, symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::element_decl_ast * val )
{
	XTHROW( x::semantic_exception, is_const_int( val->value.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::break_stat_ast * val )
{
	XTHROW( x::semantic_exception, symbols()->up_find_symbol_from_type( x::symbol_t::CYCLE ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::return_stat_ast * val )
{
	auto func_sym = symbols()->up_find_symbol_from_type( x::symbol_t::FUNCTION );
	XTHROW( x::semantic_exception, func_sym != nullptr, "" );

	XTHROW( x::semantic_exception, func_sym->cast_function()->result == get_expr_type( val->exp.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::continue_stat_ast * val )
{
	XTHROW( x::semantic_exception, symbols()->up_find_symbol_from_type( x::symbol_t::CYCLE ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::assignment_expr_ast * val )
{
	switch ( val->token )
	{
	case x::token_t::TK_ASSIGN:               // =
	case x::token_t::TK_ADD_ASSIGN:           // +=
	case x::token_t::TK_SUB_ASSIGN:           // -= 
	case x::token_t::TK_MUL_ASSIGN:           // *=
	case x::token_t::TK_DIV_ASSIGN:           // /= 
		XTHROW( x::semantic_exception, get_expr_type( val->left.get() ) == get_expr_type( val->right.get() ), "" );
		break;
	case x::token_t::TK_MOD_ASSIGN:           // %= 
	case x::token_t::TK_AND_ASSIGN:           // &= 
	case x::token_t::TK_OR_ASSIGN:            // |= 
	case x::token_t::TK_XOR_ASSIGN:           // ^= 
	case x::token_t::TK_LSHIFT_EQUAL:         // <<=
	case x::token_t::TK_RSHIFT_EQUAL:         // >>=
		XTHROW( x::semantic_exception, is_const_int( val->left.get() ) == is_const_int( val->right.get() ), "" );
		break;
	}

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::unary_expr_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::postfix_expr_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::index_expr_ast * val )
{
	XTHROW( x::semantic_exception, is_const_int( val->right.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::invoke_expr_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::member_expr_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::identifier_expr_ast * val )
{
}
