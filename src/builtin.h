#pragma once

#include "visitor.h"

namespace x
{
	class builtin : public x::visitor
	{
	public:
		virtual x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const = 0;
	};

	class builtin_is_base_of : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};

	class builtin_is_member_of : public x::builtin
	{

	};
}
