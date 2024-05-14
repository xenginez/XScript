#include "meta.h"

x::static_string_view x::meta::attribute( std::string_view key ) const
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

x::uint64 x::meta_enum::size() const
{
	return sizeof( x::int64 );
}

x::uint64 x::meta_enum::hashcode() const
{
	return x::hash( fullname() );
}

x::static_string_view x::meta_enum::name() const
{
	return _name;
}

x::static_string_view x::meta_enum::fullname() const
{
	return _fullname;
}

std::span<const x::meta_element_ptr> x::meta_enum::elements() const
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

x::uint64 x::meta_class::size() const
{
	return _size;
}

x::uint64 x::meta_class::hashcode() const
{
	return x::hash( fullname() );
}

x::static_string_view x::meta_class::name() const
{
	return _name;
}

x::static_string_view x::meta_class::fullname() const
{
	return _fullname;
}

x::static_string_view x::meta_class::base() const
{
	return _base;
}

std::span<const x::meta_variable_ptr> x::meta_class::variables() const
{
	return _variables;
}

std::span<const x::meta_function_ptr> x::meta_class::functions() const
{
	return _functions;
}

void x::meta_class::construct( void * ptr ) const
{
}

x::meta_element::meta_element()
{
}

x::meta_t x::meta_element::type() const
{
	return x::meta_t::ENUM_ELEMENT;
}

x::uint64 x::meta_element::hashcode() const
{
	return x::hash( fullname() );
}

x::static_string_view x::meta_element::name() const
{
	return _name;
}

x::static_string_view x::meta_element::fullname() const
{
	return _fullname;
}

x::int64 x::meta_element::value() const
{
	return _value;
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

x::static_string_view x::meta_variable::name() const
{
	return _name;
}

x::static_string_view x::meta_variable::fullname() const
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

x::typedesc x::meta_variable::value() const
{
	return _value;
}

void x::meta_variable::get( const x::value & obj ) const
{
}

void x::meta_variable::set( const x::value & obj ) const
{
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

x::static_string_view x::meta_function::name() const
{
	return _name;
}

x::static_string_view x::meta_function::fullname() const
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

x::typedesc x::meta_function::result() const
{
	return _result;
}

std::span<const x::meta_parameter_ptr> x::meta_function::parameters() const
{
	return _parameter_types;
}

void x::meta_function::invoke() const
{
}

x::meta_parameter::meta_parameter()
{
}

x::meta_t x::meta_parameter::type() const
{
	return x::meta_t::PARAM_ELEMENT;
}

x::uint64 x::meta_parameter::hashcode() const
{
	return x::hash( fullname() );
}

x::static_string_view x::meta_parameter::name() const
{
	return _name;
}

x::static_string_view x::meta_parameter::fullname() const
{
	return _fullname;
}

const x::typedesc & x::meta_parameter::desc() const
{
	return _type;
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

x::static_string_view x::meta_namespace::name() const
{
	return _name;
}

x::static_string_view x::meta_namespace::fullname() const
{
	return _fullname;
}

std::span<const x::meta_type_ptr> x::meta_namespace::members() const
{
	return _members;
}

x::static_string_view x::meta_attribute::find( std::string_view key ) const
{
	auto it = _map.find( key );
	if ( it != _map.end() )
		return it->second;
	return {};
}
