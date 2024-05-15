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

	class pass_with_scope : public pass
	{
	public:
		using pass::visit;

	public:
		pass_with_scope( const x::symbols_ptr & val );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

	protected:
		bool is_const_int( x::exp_stat_ast * val ) const;
		x::type_symbol * get_expr_type( x::exp_stat_ast * val ) const;
		std::string template_type_name( x::temp_type_ast * val ) const;
	};

	class symbol_scanner_pass : public pass_with_scope
	{
	public:
		using pass_with_scope::visit;

	public:
		symbol_scanner_pass( const x::symbols_ptr & val );

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

	class instant_template_pass : public pass_with_scope
	{
	public:
		using pass_with_scope::visit;

	public:
		instant_template_pass( const x::symbols_ptr & val );

	public:
		void visit( x::temp_type_ast * val ) override;

	private:
		bool matching( x::template_decl_ast * temp, x::temp_type_ast * type ) const;
		x::class_decl_ast_ptr instantiate( x::template_decl_ast * temp, x::temp_type_ast * type ) const;
	};

	class semantic_checker_pass : public pass_with_scope
	{
	public:
		using pass_with_scope::visit;

	public:
		semantic_checker_pass( const x::symbols_ptr & val );

	public:
		void visit( x::type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::array_type_ast * val ) override;

		void visit( x::element_decl_ast * val ) override;

		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;

		void visit( x::assignment_exp_ast * val ) override;
		void visit( x::unary_exp_ast * val ) override;
		void visit( x::postfix_exp_ast * val ) override;
		void visit( x::index_exp_ast * val ) override;
		void visit( x::invoke_exp_ast * val ) override;
		void visit( x::member_exp_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
	};
}
