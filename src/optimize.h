#pragma once

#include <functional>

#include "visitor.h"

namespace x
{
	class code_optimize_visitor : public x::scope_scan_visitor
	{
	public:
		using scope_scan_visitor::visit;

	public:
		class optimizer
		{
			friend class code_optimize_visitor;

		public:
			enum
			{
				MIN_LEVEL = 0,
				MID_LEVEL = 1,
				MAX_LEVEL = 2,
			};

		public:
			template<typename T> struct register_optimizer
			{
				register_optimizer()
				{
					static T _optimizer = {};
					x::code_optimize_visitor::optimizer::optimizers().push_back( &_optimizer );
				}
			};

		public:
			virtual x::uint32 level() const = 0;
			virtual std::vector<x::ast_t> ast_types() const = 0;
			virtual bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) = 0;

		private:
			static std::vector<x::code_optimize_visitor::optimizer *> & optimizers();
		};

	public:
		void optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast, x::uint32 level = 2 );

	private:
		void visit( x::closure_expr_ast * val ) override;

	private:
		x::uint32 _level = optimizer::MIN_LEVEL;
	};

	class translate_closure_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class translate_coroutine_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class constant_folding_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class constant_propagation_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class common_subexpression_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};

	class dead_code_elimination_optimizer : public x::code_optimize_visitor::optimizer
	{
	public:
		x::uint32 level() const override;
		std::vector<x::ast_t> ast_types() const override;
		bool optimize( const x::logger_ptr & logger, const x::symbols_ptr & symbols, const x::ast_ptr & ast ) override;
	};
}