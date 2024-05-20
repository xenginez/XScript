#include "ast.h"

#include "visitor.h"

x::ast_t x::unit_ast::type() const
{
	return x::ast_t::UNIT;
}

void x::unit_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::import_ast::type() const
{
	return x::ast_t::IMPORT;
}

void x::import_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::attribute_ast::type() const
{
	return x::ast_t::ATTRIBUTE;
}

void x::attribute_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::type_ast::type() const
{
	return x::ast_t::TYPE;
}

void x::type_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::temp_type_ast::type() const
{
	return x::ast_t::TEMP_TYPE;
}

void x::temp_type_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::func_type_ast::type() const
{
	return x::ast_t::FUNC_TYPE;
}

void x::func_type_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::array_type_ast::type() const
{
	return x::ast_t::ARRAY_TYPE;
}

void x::array_type_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::enum_decl_ast::type() const
{
	return x::ast_t::ENUM_DECL;
}

void x::enum_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::class_decl_ast::type() const
{
	return x::ast_t::CLASS_DECL;
}

void x::class_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::using_decl_ast::type() const
{
	return x::ast_t::USING_DECL;
}

void x::using_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::element_decl_ast::type() const
{
	return x::ast_t::ELEMENT_DECL;
}

void x::element_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::template_decl_ast::type() const
{
	return x::ast_t::TEMPLATE_DECL;
}

void x::template_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::variable_decl_ast::type() const
{
	return x::ast_t::VARIABLE_DECL;
}

void x::variable_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::function_decl_ast::type() const
{
	return x::ast_t::FUNCTION_DECL;
}

void x::function_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::parameter_decl_ast::type() const
{
	return x::ast_t::PARAMETER_DECL;
}

void x::parameter_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::namespace_decl_ast::type() const
{
	return x::ast_t::NAMESPACE_DECL;
}

void x::namespace_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::empty_stat_ast::type() const
{
	return x::ast_t::EMPTY_STAT;
}

void x::empty_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::extern_stat_ast::type() const
{
	return x::ast_t::EXTERN_STAT;
}

void x::extern_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::compound_stat_ast::type() const
{
	return x::ast_t::COMPOUND_STAT;
}

void x::compound_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::await_stat_ast::type() const
{
	return x::ast_t::AWAIT_STAT;
}

void x::await_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::yield_stat_ast::type() const
{
	return x::ast_t::YIELD_STAT;
}

void x::yield_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::new_stat_ast::type() const
{
	return x::ast_t::NEW_STAT;
}

void x::new_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::try_stat_ast::type() const
{
	return x::ast_t::TRY_STAT;
}

void x::try_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::catch_stat_ast::type() const
{
	return x::ast_t::CATCH_STAT;
}

void x::catch_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::throw_stat_ast::type() const
{
	return x::ast_t::THROW_STAT;
}

void x::throw_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::if_stat_ast::type() const
{
	return x::ast_t::IF_STAT;
}

void x::if_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::while_stat_ast::type() const
{
	return x::ast_t::WHILE_STAT;
}

void x::while_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::for_stat_ast::type() const
{
	return x::ast_t::FOR_STAT;
}

void x::for_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::foreach_stat_ast::type() const
{
	return x::ast_t::FOREACH_STAT;
}

void x::foreach_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::break_stat_ast::type() const
{
	return x::ast_t::BREAK_STAT;
}

void x::break_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::return_stat_ast::type() const
{
	return x::ast_t::RETURN_STAT;
}

void x::return_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::continue_stat_ast::type() const
{
	return x::ast_t::CONTINUE_STAT;
}

void x::continue_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::local_stat_ast::type() const
{
	return x::ast_t::LOCAL_STAT;
}

void x::local_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::assignment_expr_ast::type() const
{
	return x::ast_t::ASSIGNMENT_EXP;
}

void x::assignment_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::logical_or_expr_ast::type() const
{
	return x::ast_t::LOGICAL_OR_EXP;
}

void x::logical_or_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::logical_and_expr_ast::type() const
{
	return x::ast_t::LOGICAL_AND_EXP;
}

void x::logical_and_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::or_expr_ast::type() const
{
	return x::ast_t::OR_EXP;
}

void x::or_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::xor_expr_ast::type() const
{
	return x::ast_t::XOR_EXP;
}

void x::xor_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::and_expr_ast::type() const
{
	return x::ast_t::AND_EXP;
}

void x::and_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::compare_expr_ast::type() const
{
	return x::ast_t::COMPARE_EXP;
}

void x::compare_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::shift_expr_ast::type() const
{
	return x::ast_t::SHIFT_EXP;
}

void x::shift_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::add_expr_ast::type() const
{
	return x::ast_t::ADD_EXP;
}

void x::add_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::mul_expr_ast::type() const
{
	return x::ast_t::MUL_EXP;
}

void x::mul_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::as_expr_ast::type() const
{
	return x::ast_t::AS_EXP;
}

void x::as_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::is_expr_ast::type() const
{
	return x::ast_t::IS_EXP;
}

void x::is_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::sizeof_expr_ast::type() const
{
	return x::ast_t::SIZEOF_EXP;
}

void x::sizeof_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::typeof_expr_ast::type() const
{
	return x::ast_t::TYPEOF_EXP;
}

void x::typeof_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::unary_expr_ast::type() const
{
	return x::ast_t::UNARY_EXP;
}

void x::unary_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::postfix_expr_ast::type() const
{
	return x::ast_t::POSTFIX_EXP;
}

void x::postfix_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::index_expr_ast::type() const
{
	return x::ast_t::INDEX_EXP;
}

void x::index_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::invoke_expr_ast::type() const
{
	return x::ast_t::INVOKE_EXP;
}

void x::invoke_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::member_expr_ast::type() const
{
	return x::ast_t::MEMBER_EXP;
}

void x::member_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::identifier_expr_ast::type() const
{
	return x::ast_t::IDENTIFIER_EXP;
}

void x::identifier_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::closure_expr_ast::type() const
{
	return x::ast_t::CLOSURE_EXP;
}

void x::closure_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::arguments_expr_ast::type() const
{
	return x::ast_t::ARGUMENTS_EXP;
}

void x::arguments_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::initializers_expr_ast::type() const
{
	return x::ast_t::INITIALIZERS_EXP;
}

void x::initializers_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::null_const_expr_ast::type() const
{
	return x::ast_t::NULL_CONST_EXP;
}

void x::null_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::bool_const_expr_ast::type() const
{
	return x::ast_t::BOOL_CONST_EXP;
}

void x::bool_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::int8_const_expr_ast::type() const
{
	return x::ast_t::INT8_CONST_EXP;
}

void x::int8_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::int16_const_expr_ast::type() const
{
	return x::ast_t::INT16_CONST_EXP;
}

void x::int16_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::int32_const_expr_ast::type() const
{
	return x::ast_t::INT32_CONST_EXP;
}

void x::int32_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::int64_const_expr_ast::type() const
{
	return x::ast_t::INT64_CONST_EXP;
}

void x::int64_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::uint8_const_expr_ast::type() const
{
	return x::ast_t::UINT8_CONST_EXP;
}

void x::uint8_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::uint16_const_expr_ast::type() const
{
	return x::ast_t::UINT16_CONST_EXP;
}

void x::uint16_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::uint32_const_expr_ast::type() const
{
	return x::ast_t::UINT32_CONST_EXP;
}

void x::uint32_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::uint64_const_expr_ast::type() const
{
	return x::ast_t::UINT64_CONST_EXP;
}

void x::uint64_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::float16_const_expr_ast::type() const
{
	return x::ast_t::FLOAT16_CONST_EXP;
}

void x::float16_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::float32_const_expr_ast::type() const
{
	return x::ast_t::FLOAT32_CONST_EXP;
}

void x::float32_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::float64_const_expr_ast::type() const
{
	return x::ast_t::FLOAT64_CONST_EXP;
}

void x::float64_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::string_const_expr_ast::type() const
{
	return x::ast_t::STRING_CONST_EXP;
}

void x::string_const_expr_ast::accept( visitor * val )
{
	val->visit( this );
}
