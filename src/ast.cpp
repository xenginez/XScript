#include "ast.h"

x::ast_t x::unit_ast::type() const
{
	return x::ast_t::UNIT;
}

void x::unit_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::type_ast::type() const
{
	return x::ast_t::TYPE;
}

void x::type_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::import_ast::type() const
{
	return x::ast_t::IMPORT;
}

void x::import_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::attribute_ast::type() const
{
	return x::ast_t::ATTRIBUTE;
}

void x::attribute_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::enum_decl_ast::type() const
{
	return x::ast_t::ENUM_DECL;
}

void x::enum_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::flag_decl_ast::type() const
{
	return x::ast_t::FLAG_DECL;
}

void x::flag_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::class_decl_ast::type() const
{
	return x::ast_t::CLASS_DECL;
}

void x::class_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::using_decl_ast::type() const
{
	return x::ast_t::USING_DECL;
}

void x::using_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::enum_element_ast::type() const
{
	return x::ast_t::ENUM_ELEMENT;
}

void x::enum_element_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::flag_element_ast::type() const
{
	return x::ast_t::FLAG_ELEMENT;
}

void x::flag_element_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::template_decl_ast::type() const
{
	return x::ast_t::TEMPLATE_DECL;
}

void x::template_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::variable_decl_ast::type() const
{
	return x::ast_t::VARIABLE_DECL;
}

void x::variable_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::function_decl_ast::type() const
{
	return x::ast_t::FUNCTION_DECL;
}

void x::function_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::parameter_decl_ast::type() const
{
	return x::ast_t::PARAMETER_DECL;
}

void x::parameter_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::namespace_decl_ast::type() const
{
	return x::ast_t::NAMESPACE_DECL;
}

void x::namespace_decl_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::empty_stat_ast::type() const
{
	return x::ast_t::EMPTY_STAT;
}

void x::empty_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::extern_stat_ast::type() const
{
	return x::ast_t::EXTERN_STAT;
}

void x::extern_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::compound_stat_ast::type() const
{
	return x::ast_t::COMPOUND_STAT;
}

void x::compound_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::await_stat_ast::type() const
{
	return x::ast_t::AWAIT_STAT;
}

void x::await_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::yield_stat_ast::type() const
{
	return x::ast_t::YIELD_STAT;
}

void x::yield_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::try_stat_ast::type() const
{
	return x::ast_t::TRY_STAT;
}

void x::try_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::catch_stat_ast::type() const
{
	return x::ast_t::CATCH_STAT;
}

void x::catch_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::throw_stat_ast::type() const
{
	return x::ast_t::THROW_STAT;
}

void x::throw_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::if_stat_ast::type() const
{
	return x::ast_t::IF_STAT;
}

void x::if_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::while_stat_ast::type() const
{
	return x::ast_t::WHILE_STAT;
}

void x::while_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::for_stat_ast::type() const
{
	return x::ast_t::FOR_STAT;
}

void x::for_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::foreach_stat_ast::type() const
{
	return x::ast_t::FOREACH_STAT;
}

void x::foreach_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::break_stat_ast::type() const
{
	return x::ast_t::BREAK_STAT;
}

void x::break_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::return_stat_ast::type() const
{
	return x::ast_t::RETURN_STAT;
}

void x::return_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::continue_stat_ast::type() const
{
	return x::ast_t::CONTINUE_STAT;
}

void x::continue_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::local_stat_ast::type() const
{
	return x::ast_t::LOCAL_STAT;
}

void x::local_stat_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::assignment_exp_ast::type() const
{
	return x::ast_t::ASSIGNMENT_EXP;
}

void x::assignment_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::logical_or_exp_ast::type() const
{
	return x::ast_t::LOGICAL_OR_EXP;
}

void x::logical_or_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::logical_and_exp_ast::type() const
{
	return x::ast_t::LOGICAL_AND_EXP;
}

void x::logical_and_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::or_exp_ast::type() const
{
	return x::ast_t::OR_EXP;
}

void x::or_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::xor_exp_ast::type() const
{
	return x::ast_t::XOR_EXP;
}

void x::xor_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::and_exp_ast::type() const
{
	return x::ast_t::AND_EXP;
}

void x::and_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::compare_exp_ast::type() const
{
	return x::ast_t::COMPARE_EXP;
}

void x::compare_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::shift_exp_ast::type() const
{
	return x::ast_t::SHIFT_EXP;
}

void x::shift_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::add_exp_ast::type() const
{
	return x::ast_t::ADD_EXP;
}

void x::add_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::mul_exp_ast::type() const
{
	return x::ast_t::MUL_EXP;
}

void x::mul_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::as_exp_ast::type() const
{
	return x::ast_t::AS_EXP;
}

void x::as_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::is_exp_ast::type() const
{
	return x::ast_t::IS_EXP;
}

void x::is_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::unary_exp_ast::type() const
{
	return x::ast_t::UNARY_EXP;
}

void x::unary_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::postfix_exp_ast::type() const
{
	return x::ast_t::POSTFIX_EXP;
}

void x::postfix_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::index_exp_ast::type() const
{
	return x::ast_t::INDEX_EXP;
}

void x::index_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::invoke_exp_ast::type() const
{
	return x::ast_t::INVOKE_EXP;
}

void x::invoke_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::member_exp_ast::type() const
{
	return x::ast_t::MEMBER_EXP;
}

void x::member_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::identifier_exp_ast::type() const
{
	return x::ast_t::IDENTIFIER_EXP;
}

void x::identifier_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::closure_exp_ast::type() const
{
	return x::ast_t::CLOSURE_EXP;
}

void x::closure_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::arguments_exp_ast::type() const
{
	return x::ast_t::ARGUMENTS_EXP;
}

void x::arguments_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::initializers_exp_ast::type() const
{
	return x::ast_t::INITIALIZERS_EXP;
}

void x::initializers_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::null_const_exp_ast::type() const
{
	return x::ast_t::NULL_CONST_EXP;
}

void x::null_const_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::bool_const_exp_ast::type() const
{
	return x::ast_t::BOOL_CONST_EXP;
}

void x::bool_const_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

x::ast_t x::int8_const_exp_ast::type() const
{
	return x::ast_t::INT8_CONST_EXP;
}

void x::int8_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::int16_const_exp_ast::type() const
{
	return x::ast_t::INT16_CONST_EXP;
}

void x::int16_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::int32_const_exp_ast::type() const
{
	return x::ast_t::INT32_CONST_EXP;
}

void x::int32_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::int64_const_exp_ast::type() const
{
	return x::ast_t::INT64_CONST_EXP;
}

void x::int64_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::uint8_const_exp_ast::type() const
{
	return x::ast_t::UINT8_CONST_EXP;
}

void x::uint8_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::uint16_const_exp_ast::type() const
{
	return x::ast_t::UINT16_CONST_EXP;
}

void x::uint16_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::uint32_const_exp_ast::type() const
{
	return x::ast_t::UINT32_CONST_EXP;
}

void x::uint32_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::uint64_const_exp_ast::type() const
{
	return x::ast_t::UINT64_CONST_EXP;
}

void x::uint64_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::float16_const_exp_ast::type() const
{
	return x::ast_t::FLOAT16_CONST_EXP;
}

void x::float16_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::float32_const_exp_ast::type() const
{
	return x::ast_t::FLOAT32_CONST_EXP;
}

void x::float32_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::float64_const_exp_ast::type() const
{
	return x::ast_t::FLOAT64_CONST_EXP;
}

void x::float64_const_exp_ast::accept( ast_visitor * visitor )
{
}

x::ast_t x::string_const_exp_ast::type() const
{
	return x::ast_t::STRING_CONST_EXP;
}

void x::string_const_exp_ast::accept( ast_visitor * visitor )
{
	visitor->visit( this );
}

void x::ast_visitor::visit( x::unit_ast * val )
{
	for ( const auto & it : val->imports )
		it->accept( this );

	for ( const auto & it : val->namespaces )
		it->accept( this );
}

void x::ast_visitor::visit( x::type_ast * val )
{
}

void x::ast_visitor::visit( x::import_ast * val )
{
}

void x::ast_visitor::visit( x::attribute_ast * val )
{
}

void x::ast_visitor::visit( x::enum_decl_ast * val )
{
	for ( const auto & it : val->elements )
		it->accept( this );
}

void x::ast_visitor::visit( x::flag_decl_ast * val )
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
	val->retype->accept( this );
}

void x::ast_visitor::visit( x::enum_element_ast * val )
{
}

void x::ast_visitor::visit( x::flag_element_ast * val )
{
}

void x::ast_visitor::visit( x::template_decl_ast * val )
{
}

void x::ast_visitor::visit( x::variable_decl_ast * val )
{
	val->value_type->accept( this );
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
	val->value_type->accept( this );
}

void x::ast_visitor::visit( x::namespace_decl_ast * val )
{
	for ( const auto & it : val->members )
		it->accept( this );
}

void x::ast_visitor::visit( x::empty_stat_ast * val )
{
}

void x::ast_visitor::visit( x::extern_stat_ast * val )
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
	val->param->accept( this );
	val->body->accept( this );
}

void x::ast_visitor::visit( x::throw_stat_ast * val )
{
	val->stat->accept( this );
}

void x::ast_visitor::visit( x::if_stat_ast * val )
{
	val->cond->accept( this );
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
	val->item->accept( this );
	val->collection->accept( this );
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
	val->value_type->accept( this );

	if ( val->init ) val->init->accept( this );
}

void x::ast_visitor::visit( x::assignment_exp_ast * val )
{
	val->left->accept( this );
	val->right->accept( this );
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
	val->cast_type->accept( this );
	val->value->accept( this );
}

void x::ast_visitor::visit( x::is_exp_ast * val )
{
	val->cast_type->accept( this );
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

	val->result->accept( this );

	for ( const auto & it : val->parameters )
		it->accept( this );

	if ( val->stat ) val->stat->accept( this );
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
