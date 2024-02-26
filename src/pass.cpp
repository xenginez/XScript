#include "pass.h"

#include "context.h"
#include "symbols.h"

void x::pass::set_ctx( x::context * ctx )
{
	_ctx = ctx;
}

x::context * x::pass::context() const
{
	return _ctx;
}

x::symbols * x::pass::symbols() const
{
	return _ctx->symbols().get();
}

void x::pass::visit( x::unit_ast * val )
{
	symbols()->beg_unit();
	ast_visitor::visit( val );
	symbols()->end_unit();
}

void x::pass::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope( symbol_t::ENUM, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( symbol_t::CLASS, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( symbol_t::FUNCTION, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( symbol_t::NAMESPACE, val->name, val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::try_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::catch_stat_ast * val )
{
	symbols()->push_scope( symbol_t::BLOCK, "block_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( symbol_t::LOOP, "loop_" + std::to_string( val->location.line ) + "_" + std::to_string( val->location.column ), val );
	ast_visitor::visit( val );
	symbols()->pop_scope();
}

void x::pass::visit( x::closure_exp_ast * val )
{
	symbols()->push_scope( symbol_t::CLASS, val->name, val );
	{
		for ( const auto & it : val->captures )
			symbols()->add_symbol( symbol_t::VARIABLE, it->ident, it.get() );

		ast_visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::using_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::ALIAS, val->name, val );
	ast_visitor::visit( val );
}

void x::symbol_scanner_pass::visit( x::enum_element_ast * val )
{
	symbols()->add_symbol( symbol_t::ELEMENT, val->name, val );
	ast_visitor::visit( val );
}

void x::symbol_scanner_pass::visit( x::template_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::TEMPLATE, val->name, val );
	ast_visitor::visit( val );
}

void x::symbol_scanner_pass::visit( x::variable_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::VARIABLE, val->name, val );
	ast_visitor::visit( val );
}

void x::symbol_scanner_pass::visit( x::parameter_decl_ast * val )
{
	symbols()->add_symbol( symbol_t::PARAMETER, val->name, val );
	ast_visitor::visit( val );
}

void x::symbol_scanner_pass::visit( x::local_stat_ast * val )
{
	symbols()->add_symbol( symbol_t::LOCAL, val->name, val );
	ast_visitor::visit( val );
}

void x::reference_solver_pass::visit( x::type_ast * val )
{
	auto sym = symbols()->find_symbol( val->name );
	
	ASSERT( sym != nullptr, "" );

	symbols()->add_reference( val, sym );
}

void x::reference_solver_pass::visit( x::break_stat_ast * val )
{
	auto sym = symbols()->find_scope( x::symbol_t::LOOP );

	ASSERT( sym != nullptr, "" );

	symbols()->add_reference( val, sym );
}

void x::reference_solver_pass::visit( x::return_stat_ast * val )
{
	auto sym = symbols()->find_scope( x::symbol_t::FUNCTION );

	ASSERT( sym != nullptr, "" );

	symbols()->add_reference( val, sym );
}

void x::reference_solver_pass::visit( x::continue_stat_ast * val )
{
	auto sym = symbols()->find_scope( x::symbol_t::LOOP );

	ASSERT( sym != nullptr, "" );

	symbols()->add_reference( val, sym );
}

void x::reference_solver_pass::visit( x::identifier_exp_ast * val )
{
	auto sym = symbols()->find_symbol( val->ident );

	ASSERT( sym != nullptr, "" );

	symbols()->add_reference( val, sym );
}

void x::type_checker_pass::visit( x::assignment_exp_ast * val )
{
	ast_visitor::visit( val );

	ASSERT( exp_type( val->left.get() ) != exp_type( val->left.get() ), "" );
}

void x::type_checker_pass::visit( x::logical_or_exp_ast * val )
{
	ast_visitor::visit( val );

	auto bool_sym = symbols()->find_symbol( "bool", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != bool_sym, "" );
	ASSERT( exp_type( val->right.get() ) != bool_sym, "" );
}

void x::type_checker_pass::visit( x::logical_and_exp_ast * val )
{
	ast_visitor::visit( val );

	auto bool_sym = symbols()->find_symbol( "bool", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != bool_sym, "" );
	ASSERT( exp_type( val->right.get() ) != bool_sym, "" );
}

void x::type_checker_pass::visit( x::or_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::xor_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::and_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::compare_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::shift_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::add_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::mul_exp_ast * val )
{
	ast_visitor::visit( val );

	auto int_sym = symbols()->find_symbol( "int", symbols()->global_scope() );

	ASSERT( exp_type( val->left.get() ) != int_sym, "" );
	ASSERT( exp_type( val->right.get() ) != int_sym, "" );
}

void x::type_checker_pass::visit( x::as_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::is_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::unary_exp_ast * val )
{
	ast_visitor::visit( val );

	switch ( val->type )
	{
	case x::token_t::TK_INC:
	case x::token_t::TK_DEC:
	case x::token_t::TK_ADD:
	case x::token_t::TK_SUB:
	case x::token_t::TK_NOT:
		ASSERT( exp_type( val->exp.get() ) != symbols()->find_symbol( "int", symbols()->global_scope() ), "" );
		break;
	case x::token_t::TK_LNOT:
		ASSERT( exp_type( val->exp.get() ) != symbols()->find_symbol( "bool", symbols()->global_scope() ), "" );
		break;
	case x::token_t::TK_SIZEOF:
		break;
	}
}

void x::type_checker_pass::visit( x::postfix_exp_ast * val )
{
	ast_visitor::visit( val );

	switch ( val->type )
	{
	case x::token_t::TK_INC:
	case x::token_t::TK_DEC:
		ASSERT( exp_type( val->exp.get() ) != symbols()->find_symbol( "int", symbols()->global_scope() ), "" );
		break;
	}
}

void x::type_checker_pass::visit( x::index_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::invoke_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::member_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::identifier_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::closure_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::arguments_exp_ast * val )
{
	ast_visitor::visit( val );
}

void x::type_checker_pass::visit( x::initializers_exp_ast * val )
{
	ast_visitor::visit( val );
}

x::symbol * x::type_checker_pass::exp_type( x::exp_stat_ast * exp ) const
{
	return nullptr;
}
