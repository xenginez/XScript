#pragma once

#include <deque>

#include "visitor.h"

namespace x
{
	class semantics_analysis_visitor : public x::scope_scan_visitor
	{
	public:
		using scope_scan_visitor::visit;

	public:
		class analyzer
		{
			friend class semantics_analysis_visitor;

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
					x::semantics_analysis_visitor::analyzer::analyzers().push_back( &_analyzer );
				}
			};

		public:
			virtual x::uint32 level() const = 0;
			virtual std::vector<x::ast_t> ast_types() const = 0;
			virtual bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) = 0;

		private:
			static std::vector<x::semantics_analysis_visitor::analyzer *> & analyzers();
		};

	public:
		void analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level = 2 );

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

	class assert_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;

	private:
		void visit( x::assert_decl_ast * ast );
	};

	class access_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;

	private:
		void visit( x::identifier_expr_ast * ast );
	};

	class type_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class class_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class template_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class function_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class is_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class as_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class bool_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class sizeof_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class typeof_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class divzero_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class constant_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class condition_expr_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class deduce_variable_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class uninit_variable_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class unused_variable_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class unused_function_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class initializer_list_analyzer : public x::semantics_analysis_visitor::analyzer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool analysis( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};
}
