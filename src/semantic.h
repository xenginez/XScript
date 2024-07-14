#pragma once

#include "visitor.h"

namespace x
{
	class symbol_scanner_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		symbol_scanner_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::element_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;
	};

	class constexpr_solver_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		constexpr_solver_visitor( const x::symbols_ptr & val );

	public:
		x::const_expr_ast_ptr solver( x::expr_stat_ast * expr );
	};

	class expr_type_solver_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		expr_type_solver_visitor( const x::symbols_ptr & val );

	public:
		x::type_ast_ptr solver( x::expr_stat_ast * expr );

	public:
		void visit( x::assignment_expr_ast * val ) override;
		void visit( x::logical_or_expr_ast * val ) override;
		void visit( x::logical_and_expr_ast * val ) override;
		void visit( x::or_expr_ast * val ) override;
		void visit( x::xor_expr_ast * val ) override;
		void visit( x::and_expr_ast * val ) override;
		void visit( x::compare_expr_ast * val ) override;
		void visit( x::shift_expr_ast * val ) override;
		void visit( x::add_expr_ast * val ) override;
		void visit( x::mul_expr_ast * val ) override;
		void visit( x::as_expr_ast * val ) override;
		void visit( x::is_expr_ast * val ) override;
		void visit( x::sizeof_expr_ast * val ) override;
		void visit( x::typeof_expr_ast * val ) override;
		void visit( x::unary_expr_ast * val ) override;
		void visit( x::postfix_expr_ast * val ) override;
		void visit( x::invoke_expr_ast * val ) override;
		void visit( x::member_expr_ast * val ) override;
		void visit( x::identifier_expr_ast * val ) override;
		void visit( x::closure_expr_ast * val ) override;
		void visit( x::arguments_expr_ast * val ) override;
		void visit( x::initializers_expr_ast * val ) override;
		void visit( x::null_const_expr_ast * val ) override;
		void visit( x::bool_const_expr_ast * val ) override;
		void visit( x::int_const_expr_ast * val ) override;
		void visit( x::float_const_expr_ast * val ) override;
		void visit( x::string_const_expr_ast * val ) override;

	private:
		void push( const x::type_ast_ptr & type );
		x::type_ast_ptr pop();
		x::type_ast_ptr calc( x::token_t op, const x::type_ast_ptr & type );
		x::type_ast_ptr calc( x::token_t op, const x::type_ast_ptr & lefttype, const x::type_ast_ptr & righttype );

	private:
		std::deque<x::type_ast_ptr> _types;
	};

	class expr_value_solver_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		expr_value_solver_visitor( const x::symbols_ptr & val );

	public:
		x::ast_ptr solver( x::expr_stat_ast * expr );
	};

	class semantics_checker_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		semantics_checker_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::list_type_ast * val ) override;

		void visit( x::element_decl_ast * val ) override;

		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;

		void visit( x::assignment_expr_ast * val ) override;
		void visit( x::unary_expr_ast * val ) override;
		void visit( x::postfix_expr_ast * val ) override;
		void visit( x::invoke_expr_ast * val ) override;
		void visit( x::member_expr_ast * val ) override;
		void visit( x::identifier_expr_ast * val ) override;
	};

	class instantiate_closure_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_closure_visitor( const x::symbols_ptr & val );

	public:
		x::class_decl_ast_ptr instantiate( x::closure_expr_ast * val );
	};

	class instantiate_builtin_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_builtin_visitor( const x::symbols_ptr & val );

	public:
		x::stat_ast_ptr instantiate( x::invoke_expr_ast * val );
	};

	class instantiate_template_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_template_visitor( const x::symbols_ptr & val );

	public:
		x::class_decl_ast_ptr instantiate( x::template_decl_ast * decl, x::temp_type_ast * type );
	};

	class instantiate_coroutine_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_coroutine_visitor( const x::symbols_ptr & val );

	public:
		x::class_decl_ast_ptr instantiate( x::function_decl_ast * val );
	};

	class instantiate_initializers_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_initializers_visitor( const x::symbols_ptr & val );

	public:
		x::class_decl_ast_ptr instantiate( x::initializers_expr_ast * val );
	};

	class instantiation_constructor_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiation_constructor_visitor( const x::symbols_ptr & val );

	private:
		x::ast_ptr instantiate( x::temp_type_ast * val );
		x::ast_ptr instantiate( x::func_type_ast * val );
		x::ast_ptr instantiate( x::invoke_expr_ast * val );
		x::ast_ptr instantiate( x::closure_expr_ast * val );
		x::ast_ptr instantiate( x::function_decl_ast * val );
		x::ast_ptr instantiate( x::initializers_expr_ast * val );

	private:
		void replace( x::ast * old_ast, x::ast_ptr new_ast );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::import_ast * val ) override;
		void visit( x::attribute_ast * val ) override;

		void visit( x::type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::list_type_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::element_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::empty_stat_ast * val ) override;
		void visit( x::extern_stat_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::await_stat_ast * val ) override;
		void visit( x::yield_stat_ast * val ) override;
		void visit( x::new_stat_ast * val ) override;
		void visit( x::if_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::switch_stat_ast * val ) override;
		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;

		void visit( x::assignment_expr_ast * val ) override;
		void visit( x::logical_or_expr_ast * val ) override;
		void visit( x::logical_and_expr_ast * val ) override;
		void visit( x::or_expr_ast * val ) override;
		void visit( x::xor_expr_ast * val ) override;
		void visit( x::and_expr_ast * val ) override;
		void visit( x::compare_expr_ast * val ) override;
		void visit( x::shift_expr_ast * val ) override;
		void visit( x::add_expr_ast * val ) override;
		void visit( x::mul_expr_ast * val ) override;
		void visit( x::as_expr_ast * val ) override;
		void visit( x::is_expr_ast * val ) override;
		void visit( x::sizeof_expr_ast * val ) override;
		void visit( x::typeof_expr_ast * val ) override;
		void visit( x::unary_expr_ast * val ) override;
		void visit( x::postfix_expr_ast * val ) override;
		void visit( x::invoke_expr_ast * val ) override;
		void visit( x::member_expr_ast * val ) override;
		void visit( x::identifier_expr_ast * val ) override;
		void visit( x::closure_expr_ast * val ) override;
		void visit( x::arguments_expr_ast * val ) override;
		void visit( x::initializers_expr_ast * val ) override;
		void visit( x::null_const_expr_ast * val ) override;
		void visit( x::bool_const_expr_ast * val ) override;
		void visit( x::int_const_expr_ast * val ) override;
		void visit( x::float_const_expr_ast * val ) override;
		void visit( x::string_const_expr_ast * val ) override;
	};
}
