#include "meta.h"

#include "exception.h"

std::string_view x::meta::attribute( std::string_view key ) const
{
	if ( _attribute )
		return _attribute->find( key );
	return {};
}

x::meta_enum::meta_enum()
{
}

x::meta_t x::meta_enum::type() const
{
	return x::meta_t::ENUM;
}

x::uint64 x::meta_enum::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_enum::name() const
{
	return _name;
}

std::string_view x::meta_enum::fullname() const
{
	return _fullname;
}

x::uint64 x::meta_enum::size() const
{
	return sizeof( x::int64 );
}

std::span<const x::meta_enum::element> x::meta_enum::elements() const
{
	return _elements;
}

x::meta_class::meta_class()
{
}

x::meta_t x::meta_class::type() const
{
	return x::meta_t::CLASS;
}

x::uint64 x::meta_class::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_class::name() const
{
	return _name;
}

std::string_view x::meta_class::fullname() const
{
	return _fullname;
}

x::uint64 x::meta_class::size() const
{
	return _size;
}

const x::meta_class * x::meta_class::base() const
{
	return _base;
}

void x::meta_class::construct( void * ptr ) const
{
}

std::span<const x::meta_variable * const> x::meta_class::variables() const
{
	return _variables;
}

std::span<const x::meta_function * const> x::meta_class::functions() const
{
	return _functions;
}

std::span<const x::meta_operator * const> x::meta_class::operators() const
{
	return _operators;
}

std::span<const x::meta_interface * const> x::meta_class::interfaces() const
{
	return _interfaces;
}

x::meta_variable::meta_variable()
{
}

x::meta_t x::meta_variable::type() const
{
	return x::meta_t::VARIABLE;
}

x::uint64 x::meta_variable::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_variable::name() const
{
	return _name;
}

std::string_view x::meta_variable::fullname() const
{
	return _fullname;
}

bool x::meta_variable::is_static() const
{
	return _is_static;
}

bool x::meta_variable::is_thread() const
{
	return _is_thread;
}

x::access_t x::meta_variable::access() const
{
	return _access;
}

const x::meta_type * x::meta_variable::value_type() const
{
	return _valuetype;
}

bool x::meta_variable::get( const x::value & obj, x::value & val ) const
{
	return false;
}

bool x::meta_variable::set( const x::value & obj, const x::value & val ) const
{
	return false;
}

x::meta_function::meta_function()
{
}

x::meta_t x::meta_function::type() const
{
	return x::meta_t::FUNCTION;
}

x::uint64 x::meta_function::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_function::name() const
{
	return _name;
}

std::string_view x::meta_function::fullname() const
{
	return _fullname;
}

bool x::meta_function::is_const() const
{
	return _is_const;
}

bool x::meta_function::is_async() const
{
	return _is_async;
}

bool x::meta_function::is_static() const
{
	return _is_static;
}

x::access_t x::meta_function::access() const
{
	return _access;
}

std::span<const x::meta_type * const> x::meta_function::results() const
{
	return _results;
}

std::span<const x::meta_function::parameter> x::meta_function::parameters() const
{
	return _parameters;
}

void x::meta_function::invoke() const
{
}


x::meta_operator::meta_operator()
{
}

x::meta_t x::meta_operator::type() const
{
	return x::meta_t::FUNCTION;
}

x::uint64 x::meta_operator::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_operator::name() const
{
	return _name;
}

std::string_view x::meta_operator::fullname() const
{
	return _fullname;
}

x::access_t x::meta_operator::access() const
{
	return _access;
}

std::span<const x::meta_operator::parameter> x::meta_operator::parameters() const
{
	return _parameters;
}

std::span<const x::meta_type * const> x::meta_operator::results() const
{
	return _results;
}

void x::meta_operator::invoke() const
{
}



x::meta_interface::meta_interface()
{
}

x::meta_t x::meta_interface::type() const
{
	return x::meta_t::INTERFACE;
}

x::uint64 x::meta_interface::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_interface::name() const
{
	return _name;
}

std::string_view x::meta_interface::fullname() const
{
	return _fullname;
}

x::uint64 x::meta_interface::size() const
{
	return 0;
}

std::span<const x::meta_function * const> x::meta_interface::functions() const
{
	return _functions;
}

x::meta_namespace::meta_namespace()
{
}

x::meta_t x::meta_namespace::type() const
{
	return x::meta_t::NAMESPACE;
}

x::uint64 x::meta_namespace::hashcode() const
{
	return x::hash( fullname() );
}

std::string_view x::meta_namespace::name() const
{
	return _name;
}

std::string_view x::meta_namespace::fullname() const
{
	return _fullname;
}

std::span<const x::meta_type * const> x::meta_namespace::members() const
{
	return _members;
}

std::string_view x::meta_attribute::find( std::string_view key ) const
{
	auto it = _map.find( key );
	if ( it != _map.end() )
		return it->second;
	return {};
}
