#include "visitor.h"

#include "module.h"
#include "symbols.h"

void x::visitor::visit( x::unit_ast * val )
{
	for ( const auto & it : val->imports )
		it->accept( this );

	for ( const auto & it : val->namespaces )
		it->accept( this );
}

void x::visitor::visit( x::import_ast * val )
{
}

void x::visitor::visit( x::attribute_ast * val )
{
}

void x::visitor::visit( x::type_ast * val )
{
}

void x::visitor::visit( x::temp_type_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::visitor::visit( x::func_type_ast * val )
{
	for ( const auto & it : val->parameters )
		it->accept( this );
}

void x::visitor::visit( x::array_type_ast * val )
{
}

void x::visitor::visit( x::enum_decl_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::visitor::visit( x::class_decl_ast * val )
{
	if ( val->base )
		val->base->accept( this );
	for ( const auto & it : val->usings )
		it->accept( this );
	for ( const auto & it : val->variables )
		it->accept( this );
	for ( const auto & it : val->functions )
		it->accept( this );
}

void x::visitor::visit( x::using_decl_ast * val )
{
	val->retype->accept( this );
}

void x::visitor::visit( x::element_decl_ast * val )
{
}

void x::visitor::visit( x::template_decl_ast * val )
{
	if ( val->base )
		val->base->accept( this );
	if ( val->where )
		val->where->accept( this );

	for ( const auto & it : val->elements )
		it->accept( this );
	for ( const auto & it : val->usings )
		it->accept( this );
	for ( const auto & it : val->variables )
		it->accept( this );
	for ( const auto & it : val->functions )
		it->accept( this );
}

void x::visitor::visit( x::variable_decl_ast * val )
{
	val->value_type->accept( this );

	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::function_decl_ast * val )
{
	val->result->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	val->stat->accept( this );
}

void x::visitor::visit( x::parameter_decl_ast * val )
{
	val->value_type->accept( this );
}

void x::visitor::visit( x::namespace_decl_ast * val )
{
	for ( const auto & it : val->members )
		it->accept( this );
}

void x::visitor::visit( x::empty_stat_ast * val )
{
}

void x::visitor::visit( x::extern_stat_ast * val )
{
}

void x::visitor::visit( x::compound_stat_ast * val )
{
	for ( const auto & it : val->stats )
		it->accept( this );
}

void x::visitor::visit( x::await_stat_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::yield_stat_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::new_stat_ast * val )
{
	val->newtype->accept( this );
	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::try_stat_ast * val )
{
	val->body->accept( this );
	for ( const auto & it : val->catchs )
		it->accept( this );
}

void x::visitor::visit( x::catch_stat_ast * val )
{
	val->param->accept( this );
	val->body->accept( this );
}

void x::visitor::visit( x::throw_stat_ast * val )
{
	val->stat->accept( this );
}

void x::visitor::visit( x::if_stat_ast * val )
{
	val->cond->accept( this );
	val->then_stat->accept( this );
	if ( val->else_stat )
		val->else_stat->accept( this );
}

void x::visitor::visit( x::while_stat_ast * val )
{
	val->cond->accept( this );
	val->stat->accept( this );
}

void x::visitor::visit( x::for_stat_ast * val )
{
	val->init->accept( this );
	val->cond->accept( this );
	val->stat->accept( this );
	val->step->accept( this );
}

void x::visitor::visit( x::foreach_stat_ast * val )
{
	val->item->accept( this );
	val->collection->accept( this );
	val->stat->accept( this );
}

void x::visitor::visit( x::break_stat_ast * val )
{
}

void x::visitor::visit( x::return_stat_ast * val )
{
	if ( val->exp )
		val->exp->accept( this );
}

void x::visitor::visit( x::continue_stat_ast * val )
{
}

void x::visitor::visit( x::local_stat_ast * val )
{
	val->value_type->accept( this );

	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::assignment_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::logical_or_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::logical_and_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::or_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::xor_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::and_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::compare_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::shift_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::add_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::mul_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::as_exp_ast * val )
{
	val->cast_type->accept( this );
	val->value->accept( this );
}

void x::visitor::visit( x::is_exp_ast * val )
{
	val->cast_type->accept( this );
	val->value->accept( this );
}

void x::visitor::visit( x::sizeof_exp_ast * val )
{
	val->value->accept( this );
}

void x::visitor::visit( x::typeof_exp_ast * val )
{
	val->value->accept( this );
}

void x::visitor::visit( x::unary_exp_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::postfix_exp_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::index_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::invoke_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::member_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::identifier_exp_ast * val )
{
}

void x::visitor::visit( x::closure_exp_ast * val )
{
	for ( const auto & it : val->captures )
		it->accept( this );

	val->result->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	val->stat->accept( this );
}

void x::visitor::visit( x::arguments_exp_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::initializers_exp_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::null_const_exp_ast * val )
{
}

void x::visitor::visit( x::bool_const_exp_ast * val )
{
}

void x::visitor::visit( x::int_const_exp_ast * val )
{
}

void x::visitor::visit( x::float_const_exp_ast * val )
{
}

void x::visitor::visit( x::string_const_exp_ast * val )
{
}

x::scope_with_visitor::scope_with_visitor( const x::symbols_ptr & val )
	: _symbols( val )
{
}

void x::scope_with_visitor::visit( x::unit_ast * val )
{
	symbols()->push_scope( val->location.file );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::enum_decl_ast * val )
{
	symbols()->push_scope(val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::class_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::function_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::template_decl_ast * val )
{
	// symbols()->push_scope( val->name );
	// {
	// 	pass::visit( val );
	// }
	// symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::namespace_decl_ast * val )
{
	symbols()->push_scope( val->name );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::compound_stat_ast * val )
{
	symbols()->push_scope( std::format( "block_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::while_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::for_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

void x::scope_with_visitor::visit( x::foreach_stat_ast * val )
{
	symbols()->push_scope( std::format( "cycle_{}_{}_{}", val->location.file, val->location.line, val->location.column ) );
	{
		visitor::visit( val );
	}
	symbols()->pop_scope();
}

const x::symbols_ptr & x::scope_with_visitor::symbols() const
{
	return _symbols;
}

bool x::scope_with_visitor::is_const_int( x::exp_stat_ast * val ) const
{
	return false;
}

x::type_symbol * x::scope_with_visitor::get_expr_type( x::exp_stat_ast * val ) const
{
	return nullptr;
}

std::string x::scope_with_visitor::template_type_name( x::temp_type_ast * val ) const
{
	return std::string();
}

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

x::instantiate_visitor::instantiate_visitor( const x::symbols_ptr & val )
	: scope_with_visitor( val )
{
}

void x::instantiate_visitor::visit( x::temp_type_ast * val )
{
	//symbols()->find_template_symbols()
}

bool x::instantiate_visitor::matching( x::template_decl_ast * temp, x::temp_type_ast * type ) const
{
	return false;
}

x::class_decl_ast_ptr x::instantiate_visitor::instantiate( x::template_decl_ast * temp, x::temp_type_ast * type ) const
{
	return x::class_decl_ast_ptr();
}

x::semantic_checker_visitor::semantic_checker_visitor( const x::symbols_ptr & val )
	: scope_with_visitor( val )
{
}

void x::semantic_checker_visitor::visit( x::type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::temp_type_ast * val )
{
	//ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );
	//
	//pass_with_scope::visit( val );
}

void x::semantic_checker_visitor::visit( x::func_type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::array_type_ast * val )
{
	ASSERT( symbols()->find_type_symbol( val->name ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::element_decl_ast * val )
{
	ASSERT( is_const_int( val->value.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::break_stat_ast * val )
{
	ASSERT( symbols()->up_find_symbol_from_type( x::symbol_t::CYCLE ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::return_stat_ast * val )
{
	auto func_sym = symbols()->up_find_symbol_from_type( x::symbol_t::FUNCTION );
	ASSERT( func_sym != nullptr, "" );

	ASSERT( func_sym->cast_function()->result == get_expr_type( val->exp.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::continue_stat_ast * val )
{
	ASSERT( symbols()->up_find_symbol_from_type( x::symbol_t::CYCLE ) != nullptr, "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::assignment_exp_ast * val )
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

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::unary_exp_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::postfix_exp_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::index_exp_ast * val )
{
	ASSERT( is_const_int( val->right.get() ), "" );

	scope_with_visitor::visit( val );
}

void x::semantic_checker_visitor::visit( x::invoke_exp_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::member_exp_ast * val )
{
}

void x::semantic_checker_visitor::visit( x::identifier_exp_ast * val )
{
}

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
