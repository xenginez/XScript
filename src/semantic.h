#pragma once

#include <deque>
#include <variant>

#include "visitor.h"

namespace x
{
	class semantics_analyzer_visitor : public x::scope_scanner_visitor
	{
	public:
		using scope_scanner_visitor::visit;

	public:
		class analyzer
		{
		public:
			virtual ~analyzer() = default;

		public:
			virtual void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) = 0;
		};

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	public:
		static bool is_constant( x::ast * ast );

	private:
		std::vector<analyzer *> _analyzers;
	};

	class expression_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		using value = std::variant<std::monostate, x::ast_t, x::ast *>;

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

	private:
		x::ast_ptr evaluate_constant_expression( x::ast * ast );

	private:
		x::logger_ptr _logger;
		x::symbols_ptr _symbols;
		std::deque<value> _stack;
	};

	class div_zero_analyzer;
	class type_match_analyzer;
	class const_access_analyzer;
	class const_express_analyzer;
	class function_call_analyzer;
	class variable_uninit_analyzer;
	class variable_unused_analyzer;
}
