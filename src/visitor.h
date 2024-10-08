#pragma once

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
		virtual void visit( x::parameter_ast * val );

		virtual void visit( x::type_ast * val );
		virtual void visit( x::func_type_ast * val );
		virtual void visit( x::temp_type_ast * val );
		virtual void visit( x::list_type_ast * val );
		virtual void visit( x::array_type_ast * val );

		virtual void visit( x::enum_decl_ast * val );
		virtual void visit( x::class_decl_ast * val );
		virtual void visit( x::using_decl_ast * val );
		virtual void visit( x::template_decl_ast * val );
		virtual void visit( x::variable_decl_ast * val );
		virtual void visit( x::function_decl_ast * val );
		virtual void visit( x::interface_decl_ast * val );
		virtual void visit( x::namespace_decl_ast * val );

		virtual void visit( x::empty_stat_ast * val );
		virtual void visit( x::extern_stat_ast * val );
		virtual void visit( x::compound_stat_ast * val );
		virtual void visit( x::await_stat_ast * val );
		virtual void visit( x::yield_stat_ast * val );
		virtual void visit( x::if_stat_ast * val );
		virtual void visit( x::while_stat_ast * val );
		virtual void visit( x::for_stat_ast * val );
		virtual void visit( x::foreach_stat_ast * val );
		virtual void visit( x::switch_stat_ast * val );
		virtual void visit( x::break_stat_ast * val );
		virtual void visit( x::return_stat_ast * val );
		virtual void visit( x::try_stat_ast * val );
		virtual void visit( x::throw_stat_ast * val );
		virtual void visit( x::continue_stat_ast * val );
		virtual void visit( x::local_stat_ast * val );
		virtual void visit( x::mulocal_stat_ast * val );

		virtual void visit( x::binary_expr_ast * val );
		virtual void visit( x::unary_expr_ast * val );
		virtual void visit( x::bracket_expr_ast * val );
		virtual void visit( x::closure_expr_ast * val );
		virtual void visit( x::elements_expr_ast * val );
		virtual void visit( x::arguments_expr_ast * val );
		virtual void visit( x::identifier_expr_ast * val );
		virtual void visit( x::initializer_expr_ast * val );
		virtual void visit( x::null_constant_expr_ast * val );
		virtual void visit( x::bool_constant_expr_ast * val );
		virtual void visit( x::int_constant_expr_ast * val );
		virtual void visit( x::float_constant_expr_ast * val );
		virtual void visit( x::string_constant_expr_ast * val );
	};

	class scope_scanner_visitor : public x::visitor
	{
	public:
		using visitor::visit;

	public:
		void scanner( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::interface_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;

	protected:
		const x::logger_ptr & logger() const;
		const x::symbols_ptr & symbols() const;

	private:
		x::logger_ptr _logger;
		x::symbols_ptr _symbols;
	};

	class symbol_scanner_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void scanner( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	public:
		void visit( x::unit_ast * val ) override;

		void visit( x::parameter_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::interface_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::compound_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;
	};

	class ast_tree_printer_visitor : public x::visitor
	{
	public:
		void print( std::ostream & out, const x::ast_ptr & ast );

	public:
		void visit( x::unit_ast * val ) override;
		void visit( x::import_ast * val ) override;
		void visit( x::attribute_ast * val ) override;
		void visit( x::parameter_ast * val ) override;

		void visit( x::type_ast * val ) override;
		void visit( x::func_type_ast * val ) override;
		void visit( x::temp_type_ast * val ) override;
		void visit( x::list_type_ast * val ) override;
		void visit( x::array_type_ast * val ) override;

		void visit( x::enum_decl_ast * val ) override;
		void visit( x::class_decl_ast * val ) override;
		void visit( x::using_decl_ast * val ) override;
		void visit( x::template_decl_ast * val ) override;
		void visit( x::variable_decl_ast * val ) override;
		void visit( x::function_decl_ast * val ) override;
		void visit( x::interface_decl_ast * val ) override;
		void visit( x::namespace_decl_ast * val ) override;

		void visit( x::empty_stat_ast * val ) override;
		void visit( x::extern_stat_ast * val ) override;
		void visit( x::compound_stat_ast * val ) override;
		void visit( x::await_stat_ast * val ) override;
		void visit( x::yield_stat_ast * val ) override;
		void visit( x::if_stat_ast * val ) override;
		void visit( x::while_stat_ast * val ) override;
		void visit( x::for_stat_ast * val ) override;
		void visit( x::foreach_stat_ast * val ) override;
		void visit( x::switch_stat_ast * val ) override;
		void visit( x::break_stat_ast * val ) override;
		void visit( x::return_stat_ast * val ) override;
		void visit( x::try_stat_ast * val ) override;
		void visit( x::throw_stat_ast * val ) override;
		void visit( x::continue_stat_ast * val ) override;
		void visit( x::local_stat_ast * val ) override;
		void visit( x::mulocal_stat_ast * val ) override;

		void visit( x::binary_expr_ast * val ) override;
		void visit( x::unary_expr_ast * val ) override;
		void visit( x::identifier_expr_ast * val ) override;
		void visit( x::closure_expr_ast * val ) override;
		void visit( x::bracket_expr_ast * val ) override;
		void visit( x::elements_expr_ast * val ) override;
		void visit( x::arguments_expr_ast * val ) override;
		void visit( x::initializer_expr_ast * val ) override;
		void visit( x::null_constant_expr_ast * val ) override;
		void visit( x::bool_constant_expr_ast * val ) override;
		void visit( x::int_constant_expr_ast * val ) override;
		void visit( x::float_constant_expr_ast * val ) override;
		void visit( x::string_constant_expr_ast * val ) override;

	private:
		const char * access( x::access_t val );

	private:
		void out( std::string_view str );
		void outline( std::string_view str = {} );
		void outtab();
		void push();
		void pop();

	private:
		int _tab = 0;
		std::ostream * _cout = nullptr;
	};
}
