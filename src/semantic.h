#pragma once

#include <deque>

#include "visitor.h"

namespace x
{
	class semantics_analyzer_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	private:
		void local_uninit_checker( const x::local_stat_ast * ast );
		void variable_uninit_checker( const x::variable_decl_ast * ast );
		void array_dimension_checker( const x::binary_expr_ast * ast );
		void identifier_access_checker( const x::identifier_expr_ast * ast );
		void expression_div_zero_checker( const x::binary_expr_ast * ast );
		void novirtual_function_empty_body_checker( const x::function_decl_ast * ast );
		void virtual_function_override_final_checker( const x::function_decl_ast * ast );
		void function_parameter_default_value_checker( const x::function_decl_ast * ast );

	private:
		void throw_translate();
		void await_translate();
		void yield_translate();
		void extern_translate();
		void foreach_translate();
		void closure_translate();
		void builtin_translate();
		void invoke_virtual_translate();
		void async_function_translate();
		void array_index_to_at_translate();
		void array_to_xs_array_translate();
		void string_to_xs_string_translate();
		void variable_initializers_translate();

	private:
		bool is_constant( const x::expr_stat_ast * ast ) const;
		x::constant_expr_ast_ptr calc_constant( const x::expr_stat_ast * ast ) const;
		x::constant_expr_ast_ptr calc_unary_constant( const x::unary_expr_ast * ast ) const;
		x::constant_expr_ast_ptr calc_binary_constant( const x::binary_expr_ast * ast ) const;
		bool is_virtual_function( const x::decl_ast * owner, const x::function_decl_ast * ast ) const;

	private:
		x::template_decl_ast_ptr match_template( const x::temp_type_ast * ast ) const;

	private:
		x::logger_ptr _logger;
		x::symbols_ptr _symbols;
		std::map<std::string, int> _used;
	};
}
