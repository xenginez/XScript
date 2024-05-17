#pragma once

#include <span>

#include "runtime.h"

namespace x
{
	class interpreter : public std::enable_shared_from_this<interpreter>
	{
	public:
		bool eval( std::string_view input, x::value & result );
		bool invoke( std::string_view fullname, x::value & result, std::span<x::value> parameters );

	public:
		bool execute( const x::exp_stat_ast_ptr & ast, x::value & result );
		bool execute( const x::function_decl_ast_ptr & ast, x::value & result );
	};
}