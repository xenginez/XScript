#include "ast.h"

void x::ast_visitor::visit( x::unit_ast * val )
{
	for ( const auto & it : val->namespaces )
		it->accept( this );
}

void x::ast_visitor::visit( x::type_ast * val )
{
}

void x::ast_visitor::visit( x::enum_decl_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::ast_visitor::visit( x::class_decl_ast * val )
{
	if ( val->base ) val->base->accept( this );
	for ( const auto & it : val->variables )
		it->accept( this );
	for ( const auto & it : val->functions )
		it->accept( this );
}

void x::ast_visitor::visit( x::using_decl_ast * val )
{
	val->type->accept( this );
}

void x::ast_visitor::visit( x::enum_element_ast * val )
{
}

void x::ast_visitor::visit( x::template_decl_ast * val )
{
}

void x::ast_visitor::visit( x::variable_decl_ast * val )
{
	val->type->accept( this );
	if ( val->init ) val->init->accept( this );
}

void x::ast_visitor::visit( x::function_decl_ast * val )
{
	val->result->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	if ( val->stat ) val->stat->accept( this );
}

void x::ast_visitor::visit( x::parameter_decl_ast * val )
{
	val->type->accept( this );
}

void x::ast_visitor::visit( x::namespace_decl_ast * val )
{
	for ( const auto & it : val->members )
		it->accept( this );
}

void x::ast_visitor::visit( x::empty_stat_ast * val )
{
}

void x::ast_visitor::visit( x::compound_stat_ast * val )
{
	for ( const auto & it : val->stats )
		it->accept( this );
}

void x::ast_visitor::visit( x::await_stat_ast * val )
{
	val->exp->accept( this );
}

void x::ast_visitor::visit( x::yield_stat_ast * val )
{
	if ( val->exp ) val->exp->accept( this );
}

void x::ast_visitor::visit( x::try_stat_ast * val )
{
	val->body->accept( this );
	for ( const auto & it : val->catchs )
		it->accept( this );
}

void x::ast_visitor::visit( x::catch_stat_ast * val )
{
	val->type->accept( this );
	val->body->accept( this );
}

void x::ast_visitor::visit( x::throw_stat_ast * val )
{
	val->stat->accept( this );
}

void x::ast_visitor::visit( x::if_stat_ast * val )
{
	val->exp->accept( this );
	val->then_stat->accept( this );
	if ( val->else_stat ) val->else_stat->accept( this );
}

void x::ast_visitor::visit( x::while_stat_ast * val )
{
	val->cond->accept( this );
	val->stat->accept( this );
}

void x::ast_visitor::visit( x::for_stat_ast * val )
{
	val->init->accept( this );
	val->cond->accept( this );
	val->stat->accept( this );
	val->step->accept( this );
}

void x::ast_visitor::visit( x::foreach_stat_ast * val )
{
	val->init->accept( this );
	val->cond->accept( this );
	val->stat->accept( this );
}

void x::ast_visitor::visit( x::break_stat_ast * val )
{
}

void x::ast_visitor::visit( x::return_stat_ast * val )
{
	if ( val->exp ) val->exp->accept( this );
}

void x::ast_visitor::visit( x::continue_stat_ast * val )
{
}

void x::ast_visitor::visit( x::local_stat_ast * val )
{
	val->type->accept( this );

	if ( val->init ) val->init->accept( this );
}

void x::ast_visitor::visit( x::assignment_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::conditional_exp_ast * val )
{
	val->cond->accept( this );
	val->then_exp->accept( this );
	val->else_exp->accept( this );
}

void x::ast_visitor::visit( x::logical_or_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::logical_and_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::or_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::xor_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::and_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::compare_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::shift_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::add_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::mul_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::as_exp_ast * val )
{
	val->type->accept( this );
	val->value->accept( this );
}

void x::ast_visitor::visit( x::is_exp_ast * val )
{
	val->type->accept( this );
	val->value->accept( this );
}

void x::ast_visitor::visit( x::unary_exp_ast * val )
{
	val->exp->accept( this );
}

void x::ast_visitor::visit( x::postfix_exp_ast * val )
{
	val->exp->accept( this );
}

void x::ast_visitor::visit( x::index_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::invoke_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::member_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
}

void x::ast_visitor::visit( x::identifier_exp_ast * val )
{
}

void x::ast_visitor::visit( x::closure_exp_ast * val )
{
	for ( const auto & it : val->captures )
		it->accept( this );
	val->function->accept( this );
}

void x::ast_visitor::visit( x::arguments_exp_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::ast_visitor::visit( x::initializers_exp_ast * val )
{
	for ( const auto & it : val->args )
		it->accept( this );
}

void x::ast_visitor::visit( x::null_const_exp_ast * val )
{
}

void x::ast_visitor::visit( x::bool_const_exp_ast * val )
{
}

void x::ast_visitor::visit( x::int_const_exp_ast * val )
{
}

void x::ast_visitor::visit( x::float_const_exp_ast * val )
{
}

void x::ast_visitor::visit( x::string_const_exp_ast * val )
{
}
