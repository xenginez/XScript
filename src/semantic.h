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

	class instantiate_class_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_class_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::temp_type_ast * val ) override;

	private:
		bool matching( x::template_decl_ast * temp, x::temp_type_ast * type ) const;
		x::class_decl_ast_ptr instantiate( x::closure_expr_ast * closure ) const;
		x::class_decl_ast_ptr instantiate( x::template_decl_ast * temp, x::temp_type_ast * type ) const;
	};

	class semantic_checker_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		semantic_checker_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::array_type_ast * val ) override;

		void visit( x::element_decl_ast * val ) override;

		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;

		void visit( x::assignment_expr_ast * val ) override;
		void visit( x::unary_expr_ast * val ) override;
		void visit( x::postfix_expr_ast * val ) override;
		void visit( x::index_expr_ast * val ) override;
		void visit( x::invoke_expr_ast * val ) override;
		void visit( x::member_expr_ast * val ) override;
		void visit( x::identifier_expr_ast * val ) override;
	};
}