#pragma once

#include "visitor.h"

namespace x
{
	class semantics_analyzer_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		class div_zero_checker;
		class type_match_checker;
		class const_access_checker;
		class const_express_checker;
		class function_call_checker;
		class variable_uninit_checker;
		class variable_unused_checker;

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & val, const x::ast_ptr & ast );
	};

	class instantiate_translate_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		class instantiate_extern;
		class instantiate_builtin;
		class instantiate_closure;
		class instantiate_template;
		class instantiate_coroutine;
		class instantiate_initializers;

	public:
		void translate( const x::logger_ptr & logger, const x::symbols_ptr & val, const x::ast_ptr & ast );
	};
}
