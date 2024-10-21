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
		void local_uninit_checker();
		void variable_uninit_checker();
		void array_dimension_checker();
		void identifier_access_checker();
		void expression_div_zero_checker();
		void string_cannot_be_null_checker();
		void novirtual_function_empty_body_checker();
		void virtual_function_override_final_checker();
		void function_parameter_default_value_checker();

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
		bool is_constant();
		x::type_ast_ptr deduce_type();
		x::constant_expr_ast_ptr calc_constant();

	private:
		x::template_decl_ast_ptr match_template( const x::temp_type_ast_ptr & ast ) const;

	private:
		x::logger_ptr _logger;
		x::symbols_ptr _symbols;
		std::map<std::string, int> _used;
	};
}
