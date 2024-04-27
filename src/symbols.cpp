#include "symbols.h"

#include <regex>

x::symbols::symbols()
{
}

x::symbols::~symbols()
{
}

void x::symbols::push_unit( const x::source_location & location )
{
}

void x::symbols::pop_unit()
{
}

void x::symbols::push_scope( const x::source_location & location )
{
}

void x::symbols::add_symbol( const symbol_ptr & val )
{
}

bool x::symbols::has_symbol( std::string_view symbol_name ) const
{
	return false;
}

void x::symbols::pop_scope()
{
}

x::symbols::namespace_symbol_ptr x::symbols::global_namespace() const
{
	return namespace_symbol_ptr();
}

x::symbols::symbol_ptr x::symbols::find_symbol_from_name( std::string_view symbol_name ) const
{
	return symbol_ptr();
}

x::symbols::symbol_ptr x::symbols::find_symbol_from_fullname( std::string_view symbol_fullname ) const
{
	return symbol_ptr();
}

void x::symbols::add_reference( const x::source_location & location, const symbol_ptr & val )
{
}

x::symbols::symbol_ptr x::symbols::find_reference( const x::source_location & location ) const
{
	return symbol_ptr();
}
