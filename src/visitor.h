#pragma once

#include "ast.h"

namespace x
{
	class visitor
	{
	public:
		virtual ~visitor() = default;

	public:
		virtual void visit( x::unit_ast * val );
		virtual void visit( x::import_ast * val );
		virtual void visit( x::attribute_ast * val );

		virtual void visit( x::type_ast * val );
		virtual void visit( x::temp_type_ast * val );
		virtual void visit( x::func_type_ast * val );
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
		virtual void visit( x::try_stat_ast * val );
		virtual void visit( x::catch_stat_ast * val );
		virtual void visit( x::throw_stat_ast * val );
		virtual void visit( x::if_stat_ast * val );
		virtual void visit( x::while_stat_ast * val );
		virtual void visit( x::for_stat_ast * val );
		virtual void visit( x::foreach_stat_ast * val );
		virtual void visit( x::break_stat_ast * val );
		virtual void visit( x::return_stat_ast * val );
		virtual void visit( x::continue_stat_ast * val );
		virtual void visit( x::local_stat_ast * val );

		virtual void visit( x::assignment_exp_ast * val );
		virtual void visit( x::logical_or_exp_ast * val );
		virtual void visit( x::logical_and_exp_ast * val );
		virtual void visit( x::or_exp_ast * val );
		virtual void visit( x::xor_exp_ast * val );
		virtual void visit( x::and_exp_ast * val );
		virtual void visit( x::compare_exp_ast * val );
		virtual void visit( x::shift_exp_ast * val );
		virtual void visit( x::add_exp_ast * val );
		virtual void visit( x::mul_exp_ast * val );
		virtual void visit( x::as_exp_ast * val );
		virtual void visit( x::is_exp_ast * val );
		virtual void visit( x::sizeof_exp_ast * val );
		virtual void visit( x::typeof_exp_ast * val );
		virtual void visit( x::unary_exp_ast * val );
		virtual void visit( x::postfix_exp_ast * val );
		virtual void visit( x::index_exp_ast * val );
		virtual void visit( x::invoke_exp_ast * val );
		virtual void visit( x::member_exp_ast * val );
		virtual void visit( x::identifier_exp_ast * val );
		virtual void visit( x::closure_exp_ast * val );
		virtual void visit( x::arguments_exp_ast * val );
		virtual void visit( x::initializers_exp_ast * val );
		virtual void visit( x::null_const_exp_ast * val );
		virtual void visit( x::bool_const_exp_ast * val );
		virtual void visit( x::int_const_exp_ast * val );
		virtual void visit( x::float_const_exp_ast * val );
		virtual void visit( x::string_const_exp_ast * val );
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
		void visit( x::template_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

	protected:
		const x::symbols_ptr & symbols() const;
		bool is_const_int( x::exp_stat_ast * val ) const;
		x::type_symbol * get_expr_type( x::exp_stat_ast * val ) const;
		std::string template_type_name( x::temp_type_ast * val ) const;

	private:
		x::symbols_ptr _symbols;
	};

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

	class instantiate_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		instantiate_visitor( const x::symbols_ptr & val );

	public:
		void visit( x::temp_type_ast * val ) override;

	private:
		bool matching( x::template_decl_ast * temp, x::temp_type_ast * type ) const;
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

		void visit( x::assignment_exp_ast * val ) override;
		void visit( x::unary_exp_ast * val ) override;
		void visit( x::postfix_exp_ast * val ) override;
		void visit( x::index_exp_ast * val ) override;
		void visit( x::invoke_exp_ast * val ) override;
		void visit( x::member_exp_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
	};

	class module_scanner_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		module_scanner_visitor( const x::module_ptr & module, const x::symbols_ptr & symbols );

	public:
		void visit( x::unit_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;

	private:
		x::module_ptr _module;
	};

	class module_generater_visitor : public x::scope_with_visitor
	{
	public:
		using scope_with_visitor::visit;

	public:
		module_generater_visitor( const x::module_ptr & module, const x::symbols_ptr & symbols );

	public:
		void visit( x::attribute_ast * val ) override;

		void visit( x::type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::array_type_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::element_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::parameter_decl_ast * val ) override;

		void visit( x::extern_stat_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::await_stat_ast * val ) override;
		void visit( x::yield_stat_ast * val ) override;
		void visit( x::new_stat_ast * val ) override;
		void visit( x::try_stat_ast * val ) override;
		void visit( x::catch_stat_ast * val ) override;
		void visit( x::throw_stat_ast * val ) override;
		void visit( x::if_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;

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
		void visit( x::sizeof_exp_ast * val ) override;
		void visit( x::typeof_exp_ast * val ) override;
		void visit( x::unary_exp_ast * val ) override;
		void visit( x::postfix_exp_ast * val ) override;
		void visit( x::index_exp_ast * val ) override;
		void visit( x::invoke_exp_ast * val ) override;
		void visit( x::member_exp_ast * val ) override;
		void visit( x::identifier_exp_ast * val ) override;
		void visit( x::closure_exp_ast * val ) override;
		void visit( x::arguments_exp_ast * val ) override;
		void visit( x::initializers_exp_ast * val ) override;
		void visit( x::null_const_exp_ast * val ) override;
		void visit( x::bool_const_exp_ast * val ) override;
		void visit( x::int_const_exp_ast * val ) override;
		void visit( x::float_const_exp_ast * val ) override;
		void visit( x::string_const_exp_ast * val ) override;

	private:
		x::module_ptr _module;
	};
}