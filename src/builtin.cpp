#include "builtin.h"

x::ast_ptr x::builtin_sizeof::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}

x::ast_ptr x::builtin_typeof::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}

x::ast_ptr x::builtin_is_enum::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}

x::ast_ptr x::builtin_is_class::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}

x::ast_ptr x::builtin_is_base_of::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}

x::ast_ptr x::builtin_conditional::transferred( const x::symbols_ptr & sym, const x::ast_ptr & ast ) const
{
	return x::ast_ptr();
}
