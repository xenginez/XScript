#include "symbols.h"

#include <regex>

bool x::symbol::is_scope() const
{
	return false;
}

x::symbol_ptr x::symbol::find_child_symbol( std::string_view name ) const
{
	return symbol_ptr();
}

x::uint64 x::type_symbol::size() const
{
	return 0;
}

x::uint64 x::enum_symbol::size() const
{
	return sizeof( x::int64 );
}

x::uint64 x::flag_symbol::size() const
{
	return sizeof( x::uint64 );
}

x::uint64 x::alias_symbol::size() const
{
	return x::uint64();
}

x::uint64 x::class_symbol::size() const
{
	return x::uint64();
}

x::symbols::symbols()
{
}

x::symbols::~symbols()
{
}

void x::symbols::push_unit( const x::location & location )
{
}

void x::symbols::pop_unit()
{
}

void x::symbols::push_scope( const x::location & location )
{
}

void x::symbols::add_symbol( const x::symbol_ptr & val )
{
}

bool x::symbols::has_symbol( std::string_view symbol_name ) const
{
	return false;
}

x::symbol_ptr x::symbols::current_scope() const
{
	return x::symbol_ptr();
}

void x::symbols::pop_scope()
{
}

x::namespace_symbol_ptr x::symbols::global_namespace() const
{
	return namespace_symbol_ptr();
}

x::symbol_ptr x::symbols::find_symbol_from_name( std::string_view symbol_name ) const
{
	return symbol_ptr();
}

x::symbol_ptr x::symbols::find_symbol_from_fullname( std::string_view symbol_fullname ) const
{
	return symbol_ptr();
}

void x::symbols::add_reference( const x::location & location, const x::symbol_ptr & val )
{
}

x::symbol_ptr x::symbols::find_reference( const x::location & location ) const
{
	return symbol_ptr();
}
