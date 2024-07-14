#include "visitor.h"

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

void x::visitor::visit( x::func_type_ast * val )
{
	for ( const auto & it : val->parameters )
		it->accept( this );
}

void x::visitor::visit( x::temp_type_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::visitor::visit( x::list_type_ast * val )
{
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
//	if ( val->base )
//		val->base->accept( this );
//	if ( val->where )
//		val->where->accept( this );
//
//	for ( const auto & it : val->elements )
//		it->accept( this );
//	for ( const auto & it : val->usings )
//		it->accept( this );
//	for ( const auto & it : val->variables )
//		it->accept( this );
//	for ( const auto & it : val->functions )
//		it->accept( this );
}

void x::visitor::visit( x::variable_decl_ast * val )
{
	val->value_type->accept( this );

	if ( val->init )
		val->init->accept( this );
}

void x::visitor::visit( x::function_decl_ast * val )
{
	for ( const auto & it : val->results )
		it->accept( this );

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

void x::visitor::visit( x::switch_stat_ast * val )
{
	val->expr->accept( this );
	for ( auto & it : val->cases )
	{
		it.first->accept( this );
		it.second->accept( this );
	}
	if ( val->defult )
		val->defult->accept( this );
}

void x::visitor::visit( x::break_stat_ast * val )
{
}

void x::visitor::visit( x::return_stat_ast * val )
{
	for ( auto & it : val->exprs )
		it->accept( this );
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

void x::visitor::visit( x::assignment_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::logical_or_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::logical_and_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::or_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::xor_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::and_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::compare_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::shift_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::add_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::mul_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::as_expr_ast * val )
{
	val->cast_type->accept( this );
	val->value->accept( this );
}

void x::visitor::visit( x::is_expr_ast * val )
{
	val->cast_type->accept( this );
	val->value->accept( this );
}

void x::visitor::visit( x::sizeof_expr_ast * val )
{
	val->value->accept( this );
}

void x::visitor::visit( x::typeof_expr_ast * val )
{
	val->value->accept( this );
}

void x::visitor::visit( x::unary_expr_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::postfix_expr_ast * val )
{
	val->exp->accept( this );
}

void x::visitor::visit( x::index_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::invoke_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::member_expr_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::visitor::visit( x::typecast_expr_ast * val )
{
	val->type->accept( this );
}

void x::visitor::visit( x::identifier_expr_ast * val )
{
}

void x::visitor::visit( x::closure_expr_ast * val )
{
	for ( const auto & it : val->results )
		it->accept( this );

	for ( const auto & it : val->captures )
		it->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	val->stat->accept( this );
}

void x::visitor::visit( x::arguments_expr_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::initializers_expr_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::visitor::visit( x::null_const_expr_ast * val )
{
}

void x::visitor::visit( x::bool_const_expr_ast * val )
{
}

void x::visitor::visit( x::int_const_expr_ast * val )
{
}

void x::visitor::visit( x::float_const_expr_ast * val )
{
}

void x::visitor::visit( x::string_const_expr_ast * val )
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
