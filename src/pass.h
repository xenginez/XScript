#pragma once

#include "ast.h"

namespace x
{
	class pass : public ast_visitor
	{
	public:
		using ast_visitor::visit;

	public:
		pass( const x::symbols_ptr & val );

	public:
		const x::symbols_ptr & symbols() const;

	private:
		x::symbols_ptr _symbols;
	};

	class symbol_scanner_pass : public pass
	{
	public:
		using pass::visit;

	public:
		symbol_scanner_pass( const x::symbols_ptr & val );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::flag_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::enum_element_ast * val ) override;
		void visit( x::flag_element_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;
	};

	class reference_solver_pass : public pass
	{
	public:
		using pass::visit;

	public:
		reference_solver_pass( const x::symbols_ptr & val );

	public:
		void visit( x::enum_element_ast * val ) override;
		void visit( x::flag_element_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
	};

	class type_checker_pass : public pass
	{
	public:
		using pass::visit;

	public:
		type_checker_pass( const x::symbols_ptr & val );

	public:
		void visit( x::assignment_exp_ast * val ) override;
		void visit( x::logical_or_exp_ast * val ) override;
		void visit( x::logical_and_exp_ast * val ) override;
		void visit( x::or_exp_ast * val ) override;
		void visit( x::xor_exp_ast * val ) override;
		void visit( x::and_exp_ast * val ) override;
		void visit( x::compare_exp_ast * val ) override;
		void visit( x::shift_exp_ast * val ) override;
		void visit( x::add_exp_ast * val ) override;
		void visit( x::mul_exp_ast * val ) override;
		void visit( x::as_exp_ast * val ) override;
		void visit( x::is_exp_ast * val ) override;
		void visit( x::unary_exp_ast * val ) override;
		void visit( x::postfix_exp_ast * val ) override;
		void visit( x::index_exp_ast * val ) override;
		void visit( x::invoke_exp_ast * val ) override;
		void visit( x::member_exp_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;
		void visit( x::arguments_exp_ast * val ) override;
		void visit( x::initializers_exp_ast * val ) override;

	public:
		x::type_symbol_ptr expr_type( const x::ast_ptr & ast ) const;
		x::type_symbol_ptr calc_type( x::token_t oper, const x::ast_ptr & left, const x::ast_ptr & right = nullptr ) const;
	};

	class semantic_checker_pass : public pass
	{
	public:
		using pass::visit;

	public:
		semantic_checker_pass( const x::symbols_ptr & val );

	public:
		void visit( x::if_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

	private:
		x::type_symbol_ptr expr_type( const x::ast_ptr & ast ) const;
	};
}
