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

void x::symbol_scanner_pass::visit( x::unit_ast * val )
{
	symbols()->push_unit( val->location );
	pass::visit( val );
	symbols()->pop_unit();
}

void x::symbol_scanner_pass::visit( x::enum_decl_ast * val )
{
	auto symbol = std::make_shared<x::enum_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::flag_decl_ast * val )
{
	auto symbol = std::make_shared<x::flag_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::class_decl_ast * val )
{
	auto symbol = std::make_shared<x::class_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::using_decl_ast * val )
{
	auto symbol = std::make_shared<x::alias_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::enum_element_ast * val )
{
	auto symbol = std::make_shared<x::enum_element_symbol>();
	symbol->name = val->name;
	symbol->value = val->value;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::flag_element_ast * val )
{
	auto symbol = std::make_shared<x::flag_element_symbol>();
	symbol->name = val->name;
	symbol->value = val->value;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::template_decl_ast * val )
{
	auto symbol = std::make_shared<x::template_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::variable_decl_ast * val )
{
	auto symbol = std::make_shared<x::variable_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::function_decl_ast * val )
{
	auto symbol = std::make_shared<x::function_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::parameter_decl_ast * val )
{
	auto symbol = std::make_shared<x::variable_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::namespace_decl_ast * val )
{
	auto symbol = std::make_shared<x::namespace_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::compound_stat_ast * val )
{
	auto symbol = std::make_shared<x::block_symbol>();
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::symbol_scanner_pass::visit( x::local_stat_ast * val )
{
	auto symbol = std::make_shared<x::variable_symbol>();
	symbol->name = val->name;
	symbol->location = val->location;
	symbols()->add_symbol( symbol );

	pass::visit( val );
}

void x::symbol_scanner_pass::visit( x::closure_exp_ast * val )
{
	symbols()->push_scope( val->location );
	pass::visit( val );
	symbols()->pop_scope();
}

void x::reference_solver_pass::visit( x::enum_element_ast * val )
{
	symbols()->add_reference( val->location, symbols()->current_scope() );
}

void x::reference_solver_pass::visit( x::flag_element_ast * val )
{
	symbols()->add_reference( val->location, symbols()->current_scope() );
}

void x::reference_solver_pass::visit( x::identifier_exp_ast * val )
{
	auto sym = symbols()->find_symbol_from_name( val->ident );

	ASSERT( sym, "" );

	symbols()->add_reference( val->location, sym );
}

void x::type_checker_pass::visit( x::if_stat_ast * val )
{
}

void x::type_checker_pass::visit( x::while_stat_ast * val )
{
}

void x::type_checker_pass::visit( x::for_stat_ast * val )
{
}

void x::type_checker_pass::visit( x::foreach_stat_ast * val )
{
}

void x::type_checker_pass::visit( x::assignment_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::conditional_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::logical_or_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::logical_and_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::or_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::xor_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::and_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::compare_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::shift_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::add_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::mul_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::as_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::is_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::unary_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::postfix_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::index_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::invoke_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::member_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::identifier_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::closure_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::arguments_exp_ast * val )
{
}

void x::type_checker_pass::visit( x::initializers_exp_ast * val )
{
}

x::symbol_ptr x::type_checker_pass::expr_type( const x::ast_ptr & ast ) const
{
	return x::symbol_ptr();
}

bool x::type_checker_pass::check_type( const x::symbol_ptr & left, const x::symbol_ptr & right ) const
{
	x::type_symbol_ptr left_type, right_type;

	switch ( left->type )
	{
	case x::symbol_t::ENUM:
		left_type = std::static_pointer_cast<x::type_symbol>( left );
		break;
	case x::symbol_t::LOOP:
		break;
	case x::symbol_t::BLOCK:
		break;
	case x::symbol_t::ALIAS:
		break;
	case x::symbol_t::PARAM:
		break;
	case x::symbol_t::LOCAL:
		break;
	case x::symbol_t::CLASS:
		left_type = std::static_pointer_cast<x::type_symbol>( left );
		break;
	case x::symbol_t::ELEMENT:
		break;
	case x::symbol_t::TEMPLATE:
		break;
	case x::symbol_t::VARIABLE:
		break;
	case x::symbol_t::FUNCTION:
		break;
	case x::symbol_t::NAMESPACE:
		break;
	default:
		break;
	}

	return false;
}
