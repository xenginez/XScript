#include "meta.h"

#include "runtime.h"

x::meta_t x::meta::type() const
{
	return _type;
}

uint64_t x::meta::hashcode() const
{
	return _hashcode;
}

x::static_string_view x::meta::name() const
{
	return _name;
}

x::static_string_view x::meta::fullname() const
{
	return _fullname;
}

x::meta_namespace::meta_namespace()
{
	meta::_type = meta_t::NAMESPACE;
}

std::span<x::meta_ptr> x::meta_namespace::members() const
{
	return _members;
}

x::meta_function::meta_function()
{
	meta::_type = meta_t::FUNCTION;
}

x::access_t x::meta_function::access() const
{
	return _access;
}

x::modify_flag x::meta_function::modify() const
{
	return _modify;
}

x::type_desc x::meta_function::result_type() const
{
	return _result_type;
}

std::span<x::type_desc> x::meta_function::parameter_types() const
{
	return _parameter_types;
}

x::meta_variable::meta_variable()
{
	meta::_type = meta_t::VARIABLE;
}

x::access_t x::meta_variable::access() const
{
	return _access;
}

x::modify_flag x::meta_variable::modify() const
{
	return _modify;
}

x::type_desc x::meta_variable::value_type() const
{
	return _value_type;
}

x::meta_class::meta_class()
{
	meta::_type = meta_t::CLASS;
}

uint64_t x::meta_class::class_size() const
{
	return _size;
}

x::static_string_view x::meta_class::class_base() const
{
	return _base;
}

std::span<x::meta_variable_ptr> x::meta_class::variables() const
{
	return _variables;
}

std::span<x::meta_function_ptr> x::meta_class::functions() const
{
	return _functions;
}

x::meta_enum::meta_enum()
{
	meta::_type = meta_t::ENUM;
}

std::span<std::pair<x::static_string_view, x::value>> x::meta_enum::elements() const
{
	return _elements;
}

x::meta_extern_function::meta_extern_function()
{
	meta::_type = x::meta_t::EXTERN_FUNCTION;
}

void x::meta_extern_function::invoke() const
{
	runtime::exec_extern( _lib, _proc );
}

x::meta_script_function::meta_script_function()
{
	meta::_type = x::meta_t::SCRIPT_FUNCTION;
}

void x::meta_script_function::invoke() const
{
	switch ( _code.type )
	{
	case x::code_t::IR:
		runtime::exec_ir( _code.idx );
		break;
	case x::code_t::JIT:
		runtime::exec_jit( _code.idx );
		break;
	case x::code_t::AOT:
		runtime::exec_aot( _code.idx );
		break;
	}
}

x::meta_script_variable::meta_script_variable()
{
	meta::_type = x::meta_t::SCRIPT_VARIABLE;
}

void x::meta_script_variable::get() const
{
	if ( (int)modify() & (int)modify_flag::STATIC )
	{
		runtime::push( runtime::global( _idx ) );
	}
	else if ( (int)modify() & (int)modify_flag::THREAD )
	{
		runtime::push( runtime::thread( _idx ) );
	}
	else
	{

	}
}

void x::meta_script_variable::set() const
{
}

x::meta_script_class::meta_script_class()
{
	meta::_type = x::meta_t::SCRIPT_CLASS;
}

void x::meta_script_class::construct( void * ptr ) const
{
	runtime::push( new x::script_object( hashcode(), ptr ) );

	switch ( _code.type )
	{
	case x::code_t::IR:
		runtime::exec_ir( _code.idx );
		break;
	case x::code_t::JIT:
		runtime::exec_jit( _code.idx );
		break;
	case x::code_t::AOT:
		runtime::exec_aot( _code.idx );
		break;
	}

	runtime::pop();
}

x::meta_script_enum::meta_script_enum()
{
	meta::_type = x::meta_t::SCRIPT_ENUM;
}
