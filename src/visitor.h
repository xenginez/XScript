#pragma once

#include <deque>

#include "ast.h"

namespace x
{
	class visitor : public std::enable_shared_from_this<visitor>
	{
	public:
		virtual ~visitor() = default;

	public:
		virtual void visit( x::unit_ast * val );
		virtual void visit( x::import_ast * val );
		virtual void visit( x::attribute_ast * val );

		virtual void visit( x::type_ast * val );
		virtual void visit( x::func_type_ast * val );
		virtual void visit( x::temp_type_ast * val );
		virtual void visit( x::list_type_ast * val );
		virtual void visit( x::array_type_ast * val );

		virtual void visit( x::enum_decl_ast * val );
		virtual void visit( x::class_decl_ast * val );
		virtual void visit( x::using_decl_ast * val );
		virtual void visit( x::element_decl_ast * val );
		virtual void visit( x::template_decl_ast * val );
		virtual void visit( x::variable_decl_ast * val );
		virtual void visit( x::function_decl_ast * val );
		virtual void visit( x::parameter_decl_ast * val );
		virtual void visit( x::namespace_decl_ast * val );

		virtual void visit( x::empty_stat_ast * val );
		virtual void visit( x::extern_stat_ast * val );
		virtual void visit( x::compound_stat_ast * val );
		virtual void visit( x::await_stat_ast * val );
		virtual void visit( x::yield_stat_ast * val );
		virtual void visit( x::new_stat_ast * val );
		virtual void visit( x::if_stat_ast * val );
		virtual void visit( x::while_stat_ast * val );
		virtual void visit( x::for_stat_ast * val );
		virtual void visit( x::foreach_stat_ast * val );
		virtual void visit( x::switch_stat_ast * val );
		virtual void visit( x::break_stat_ast * val );
		virtual void visit( x::return_stat_ast * val );
		virtual void visit( x::continue_stat_ast * val );
		virtual void visit( x::local_stat_ast * val );

		virtual void visit( x::assignment_expr_ast * val );
		virtual void visit( x::logical_or_expr_ast * val );
		virtual void visit( x::logical_and_expr_ast * val );
		virtual void visit( x::or_expr_ast * val );
		virtual void visit( x::xor_expr_ast * val );
		virtual void visit( x::and_expr_ast * val );
		virtual void visit( x::compare_expr_ast * val );
		virtual void visit( x::shift_expr_ast * val );
		virtual void visit( x::add_expr_ast * val );
		virtual void visit( x::mul_expr_ast * val );
		virtual void visit( x::as_expr_ast * val );
		virtual void visit( x::is_expr_ast * val );
		virtual void visit( x::sizeof_expr_ast * val );
		virtual void visit( x::typeof_expr_ast * val );
		virtual void visit( x::unary_expr_ast * val );
		virtual void visit( x::postfix_expr_ast * val );
		virtual void visit( x::index_expr_ast * val );
		virtual void visit( x::invoke_expr_ast * val );
		virtual void visit( x::member_expr_ast * val );
		virtual void visit( x::typecast_expr_ast * val );
		virtual void visit( x::identifier_expr_ast * val );
		virtual void visit( x::closure_expr_ast * val );
		virtual void visit( x::arguments_expr_ast * val );
		virtual void visit( x::initializers_expr_ast * val );
		virtual void visit( x::null_const_expr_ast * val );
		virtual void visit( x::bool_const_expr_ast * val );
		virtual void visit( x::int_const_expr_ast * val );
		virtual void visit( x::float_const_expr_ast * val );
		virtual void visit( x::string_const_expr_ast * val );
	};

	class scope_with_visitor : public x::visitor
	{
	public:
		using visitor::visit;

	public:
		scope_with_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

	protected:
		const x::symbols_ptr & symbols() const;

	private:
		x::symbols_ptr _symbols;
	};
}
