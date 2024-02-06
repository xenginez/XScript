#include "value.h"

x::value::value()
	: _type( value_t::INVALID ), _object( nullptr )
{
}

x::value::value( value && val )
	: _type( value_t::INVALID ), _object( nullptr )
{
	swap( std::forward<value &>( val ) );
}

x::value::value( const value & val )
	: _type( val._type ), _object( val._object )
{
}

x::value & x::value::operator=( value && val )
{
	swap( std::forward<value &>( val ) );

	return *this;
}

x::value & x::value::operator=( const value & val )
{
	_type = val._type;
	_object = val._object;

	return *this;
}

void x::value::swap( value & val )
{
	std::swap( _type, val._type );
	std::swap( _object, val._object );
}

x::value_t x::value::type() const
{
	return _type;
}
