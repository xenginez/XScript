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
		class analyzer
		{
			friend class semantics_analyzer_visitor;

		public:
			enum
			{
				MIN_LEVEL = 0,
				MID_LEVEL = 1,
				MAX_LEVEL = 2,
			};

		public:
			template<typename T> struct register_analyzer
			{
				register_analyzer()
				{
					static T _analyzer = {};
					x::semantics_analyzer_visitor::analyzer::analyzers().push_back( &_analyzer );
				}
			};

		public:
			virtual x::uint32 level() const = 0;
			virtual std::vector<x::ast_t> support_ast_types() const = 0;
			virtual bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast );

		private:
			static std::vector<x::semantics_analyzer_visitor::analyzer *> & analyzers();
		};

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level = 2 );

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

	private:
		bool is_virtual_function( const x::decl_ast * owner, const x::function_decl_ast * ast ) const;

	private:
		x::template_decl_ast_ptr match_template( const x::temp_type_ast * ast ) const;

	private:
		x::logger_ptr _logger;
		x::symbols_ptr _symbols;
		std::map<std::string, int> _used;
	};

	class assert_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class access_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class express_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class type_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class class_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class template_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class function_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class is_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class as_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class bool_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class sizeof_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class typeof_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class condition_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class exception_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class any_variable_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class array_out_range_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class uninit_variable_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class unused_variable_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class unused_function_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class initializer_list_analyzer : public x::semantics_analyzer_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> support_ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};
	
}
