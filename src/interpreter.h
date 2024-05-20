#pragma once

#include <span>

#include "runtime.h"

namespace x
{
	class interpreter : public std::enable_shared_from_this<interpreter>
	{
	public:
		bool eval( const x::runtime_ptr & rt, std::string_view input, x::value & result );
		bool invoke( const x::runtime_ptr & rt, std::string_view fullname, x::value & result, std::span<x::value> parameters );

	public:
		bool execute_express( const x::runtime_ptr & rt, const x::expr_stat_ast_ptr & ast, x::value & result );
		bool execute_statement( const x::runtime_ptr & rt, const x::stat_ast_ptr & ast, x::value & result );
		bool execute_function( const x::runtime_ptr & rt, const x::function_decl_ast_ptr & ast, x::value & result );
	};
}