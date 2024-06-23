#pragma once

#include "visitor.h"

namespace x
{
	class builtin : public x::visitor
	{
	public:
		virtual x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const = 0;
	};

	// builtin_sizeof( type | var )
	class builtin_sizeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// builtin_typeof( type | var )
	class builtin_typeof : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// builtin_is_enum( type )
	class builtin_is_enum : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// builtin_is_class( type )
	class builtin_is_class : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// builtin_is_base_of( type, base_type )
	class builtin_is_base_of : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
	// builtin_conditional( cond, then_type, else_type )
	class builtin_conditional : public x::builtin
	{
	public:
		x::ast_ptr transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const override;
	};
}
