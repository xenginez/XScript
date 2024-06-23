#pragma once

#include "type.h"
#include "visitor.h"

namespace x
{
	class interpreter : public x::visitor
	{
	public:
		bool eval( const x::runtime_ptr & rt, std::string_view code, x::value & result );
		bool invoke( const x::runtime_ptr & rt, std::string_view fullname, x::value & result, std::span<x::value> parameters );

	public:
		bool execute_express( const x::runtime_ptr & rt, const x::expr_stat_ast_ptr & ast, x::value & result );
		bool execute_statement( const x::runtime_ptr & rt, const x::stat_ast_ptr & ast, x::value & result );
		bool execute_function( const x::runtime_ptr & rt, const x::function_decl_ast_ptr & ast, x::value & result );

	private:
		x::symbols_ptr _symbols;
	};
}