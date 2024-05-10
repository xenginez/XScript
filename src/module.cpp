#include "module.h"

x::module::module()
{
}

void x::module::generate( const x::symbols_ptr & symbols, const x::unit_ast_ptr & unit )
{
	_symbols = symbols.get();
	unit->accept( this );
	_symbols = nullptr;
}

void x::module::visit( x::unit_ast * val )
{
}

void x::module::visit( x::type_ast * val )
{
}

void x::module::visit( x::import_ast * val )
{
}

void x::module::visit( x::attribute_ast * val )
{
}

void x::module::visit( x::enum_decl_ast * val )
{
}

void x::module::visit( x::flag_decl_ast * val )
{
}

void x::module::visit( x::class_decl_ast * val )
{
}

void x::module::visit( x::using_decl_ast * val )
{
}

void x::module::visit( x::enum_element_ast * val )
{
}

void x::module::visit( x::flag_element_ast * val )
{
}

void x::module::visit( x::template_decl_ast * val )
{
}

void x::module::visit( x::variable_decl_ast * val )
{
}

void x::module::visit( x::function_decl_ast * val )
{
}

void x::module::visit( x::parameter_decl_ast * val )
{
}

void x::module::visit( x::namespace_decl_ast * val )
{
}

void x::module::visit( x::empty_stat_ast * val )
{
}

void x::module::visit( x::extern_stat_ast * val )
{
}

void x::module::visit( x::compound_stat_ast * val )
{
}

void x::module::visit( x::await_stat_ast * val )
{
}

void x::module::visit( x::yield_stat_ast * val )
{
}

void x::module::visit( x::try_stat_ast * val )
{
}

void x::module::visit( x::catch_stat_ast * val )
{
}

void x::module::visit( x::throw_stat_ast * val )
{
}

void x::module::visit( x::if_stat_ast * val )
{
}

void x::module::visit( x::while_stat_ast * val )
{
}

void x::module::visit( x::for_stat_ast * val )
{
}

void x::module::visit( x::foreach_stat_ast * val )
{
}

void x::module::visit( x::break_stat_ast * val )
{
}

void x::module::visit( x::return_stat_ast * val )
{
}

void x::module::visit( x::continue_stat_ast * val )
{
}

void x::module::visit( x::local_stat_ast * val )
{
}

void x::module::visit( x::assignment_exp_ast * val )
{
}

void x::module::visit( x::logical_or_exp_ast * val )
{
}

void x::module::visit( x::logical_and_exp_ast * val )
{
}

void x::module::visit( x::or_exp_ast * val )
{
}

void x::module::visit( x::xor_exp_ast * val )
{
}

void x::module::visit( x::and_exp_ast * val )
{
}

void x::module::visit( x::compare_exp_ast * val )
{
}

void x::module::visit( x::shift_exp_ast * val )
{
}

void x::module::visit( x::add_exp_ast * val )
{
}

void x::module::visit( x::mul_exp_ast * val )
{
}

void x::module::visit( x::as_exp_ast * val )
{
}

void x::module::visit( x::is_exp_ast * val )
{
}

void x::module::visit( x::sizeof_exp_ast * val )
{
}

void x::module::visit( x::typeof_exp_ast * val )
{
}

void x::module::visit( x::unary_exp_ast * val )
{
}

void x::module::visit( x::postfix_exp_ast * val )
{
}

void x::module::visit( x::index_exp_ast * val )
{
}

void x::module::visit( x::invoke_exp_ast * val )
{
}

void x::module::visit( x::member_exp_ast * val )
{
}

void x::module::visit( x::identifier_exp_ast * val )
{
}

void x::module::visit( x::closure_exp_ast * val )
{
}

void x::module::visit( x::arguments_exp_ast * val )
{
}

void x::module::visit( x::initializers_exp_ast * val )
{
}

void x::module::visit( x::null_const_exp_ast * val )
{
}

void x::module::visit( x::bool_const_exp_ast * val )
{
}

void x::module::visit( x::int_const_exp_ast * val )
{
}

void x::module::visit( x::float_const_exp_ast * val )
{
}

void x::module::visit( x::string_const_exp_ast * val )
{
}
