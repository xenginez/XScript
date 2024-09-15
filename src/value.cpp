#include "value.h"

#include "object.h"
#include "runtime.h"
#include "exception.h"

x::value::value()
{
	obj = nullptr;
}

x::value::value( const value & val )
	: _flags( val._flags ), obj( val.obj )
{
}

x::value & x::value::operator=( const value & val )
{
	_flags = val._flags;
	obj = val.obj;
	return *this;
}

x::value::~value()
{
}

x::value::value( bool val )
{
	b = val;
	_flags = x::value_t::BOOL;
}

x::value::value( x::int8 val )
{
	i8 = val;
	_flags = x::value_t::INT8;
}

x::value::value( x::int16 val )
{
	i16 = val;
	_flags = x::value_t::INT16;
}

x::value::value( x::int32 val )
{
	i32 = val;
	_flags = x::value_t::INT32;
}

x::value::value( x::int64 val )
{
	i64 = val;
	_flags = x::value_t::INT64;
}

x::value::value( x::uint8 val )
{
	u8 = val;
	_flags = x::value_t::UINT8;
}

x::value::value( x::uint16 val )
{
	u16 = val;
	_flags = x::value_t::UINT16;
}

x::value::value( x::uint32 val )
{
	u32 = val;
	_flags = x::value_t::UINT32;
}

x::value::value( x::uint64 val )
{
	u64 = val;
	_flags = x::value_t::UINT64;
}

x::value::value( x::float16 val )
{
	f16 = val;
	_flags = x::value_t::FLOAT16;
}

x::value::value( x::float32 val )
{
	f32 = val;
	_flags = x::value_t::FLOAT32;
}

x::value::value( x::float64 val )
{
	f64 = val;
	_flags = x::value_t::FLOAT64;
}

x::value::value( x::string val )
{
	str = val;
	_flags = x::value_t::STRING;
}

x::value::value( x::intptr val )
{
	ptr = val;
	_flags = x::value_t::INTPTR;
}

x::value::value( x::value * val )
{
	ref = val;
	_flags = x::value_t::REFERENCE;
}

x::value::value( x::object * val )
{
	obj = val;
	_flags = x::value_t::OBJECT;
}

x::value::value( x::value_flags val )
	: _flags( val )
{
	obj = nullptr;
}

x::value & x::value::operator=( bool val )
{
	b = val;
	_flags = x::value_t::BOOL;
	return *this;
}

x::value & x::value::operator=( x::int8 val )
{
	i8 = val;
	_flags = x::value_t::INT8;
	return *this;
}

x::value & x::value::operator=( x::int16 val )
{
	i16 = val;
	_flags = x::value_t::INT16;
	return *this;
}

x::value & x::value::operator=( x::int32 val )
{
	i32 = val;
	_flags = x::value_t::INT32;
	return *this;
}

x::value & x::value::operator=( x::int64 val )
{
	i64 = val;
	_flags = x::value_t::INT64;
	return *this;
}

x::value & x::value::operator=( x::uint8 val )
{
	u8 = val;
	_flags = x::value_t::UINT8;
	return *this;
}

x::value & x::value::operator=( x::uint16 val )
{
	u16 = val;
	_flags = x::value_t::UINT16;
	return *this;
}

x::value & x::value::operator=( x::uint32 val )
{
	u32 = val;
	_flags = x::value_t::UINT32;
	return *this;
}

x::value & x::value::operator=( x::uint64 val )
{
	u64 = val;
	_flags = x::value_t::UINT64;
	return *this;
}

x::value & x::value::operator=( x::float16 val )
{
	f16 = val;
	_flags = x::value_t::FLOAT16;
	return *this;
}

x::value & x::value::operator=( x::float32 val )
{
	f32 = val;
	_flags = x::value_t::FLOAT32;
	return *this;
}

x::value & x::value::operator=( x::float64 val )
{
	f64 = val;
	_flags = x::value_t::FLOAT64;
	return *this;
}

x::value & x::value::operator=( x::string val )
{
	str = val;
	_flags = x::value_t::STRING;
	return *this;
}

x::value & x::value::operator=( x::intptr val )
{
	ptr = val;
	_flags = x::value_t::INTPTR;
	return *this;
}

x::value & x::value::operator=( x::value * val )
{
	ref = val;
	_flags = x::value_t::REFERENCE;
	return *this;
}

x::value & x::value::operator=( x::object * val )
{
	obj = val;
	_flags = x::value_t::OBJECT;
	return *this;
}

x::value_t x::value::type() const
{
	if ( to_ref()._flags == x::value_t::INVALID )
		return x::value_t::INVALID;
	else if ( to_ref()._flags == x::value_t::NIL )
		return x::value_t::NIL;
	else
		return ( to_ref()._flags & x::value_t::TYPE_MASK ).value();
}

x::value_flags x::value::flags() const
{
	return _flags;
}

bool x::value::empty() const
{
	return _flags == x::value_t::INVALID;
}

bool x::value::is_ref() const
{
	return _flags == x::value_t::REFERENCE;
}

bool x::value::is_null() const
{
	return type() == x::value_t::NIL;
}

bool x::value::is_bool() const
{
	return type() == x::value_t::BOOL;
}

bool x::value::is_int8() const
{
	return type() == x::value_t::INT8;
}

bool x::value::is_int16() const
{
	return type() == x::value_t::INT16;
}

bool x::value::is_int32() const
{
	return type() == x::value_t::INT32;
}

bool x::value::is_int64() const
{
	return type() == x::value_t::INT64;
}

bool x::value::is_signed() const
{
	return to_ref()._flags || x::value_t::SIGNED_MASK;
}

bool x::value::is_uint8() const
{
	return type() == x::value_t::UINT8;
}

bool x::value::is_uint16() const
{
	return type() == x::value_t::UINT16;
}

bool x::value::is_uint32() const
{
	return type() == x::value_t::UINT32;
}

bool x::value::is_uint64() const
{
	return type() == x::value_t::UINT64;
}

bool x::value::is_unsigned() const
{
	return to_ref()._flags || x::value_t::UNSIGNED_MASK;
}

bool x::value::is_float16() const
{
	return type() == x::value_t::FLOAT16;
}

bool x::value::is_float32() const
{
	return type() == x::value_t::FLOAT32;
}

bool x::value::is_float64() const
{
	return type() == x::value_t::FLOAT64;
}

bool x::value::is_floating() const
{
	return to_ref()._flags || x::value_t::FLOATING_MASK;
}

bool x::value::is_string() const
{
	return type() == x::value_t::STRING;
}

bool x::value::is_intptr() const
{
	return type() == x::value_t::INTPTR;
}

bool x::value::is_object() const
{
	return type() == x::value_t::OBJECT;
}

bool x::value::to_bool() const
{
	XTHROW( x::bad_value_access, !is_bool(), "" );

	return to_ref().b;
}

x::int8 x::value::to_int8() const
{
	XTHROW( x::bad_value_access, !is_int8(), "" );

	return to_ref().i8;
}

x::int16 x::value::to_int16() const
{
	XTHROW( x::bad_value_access, !is_int16(), "" );

	return to_ref().i16;
}

x::int32 x::value::to_int32() const
{
	XTHROW( x::bad_value_access, !is_int32(), "" );

	return to_ref().i32;
}

x::int64 x::value::to_int64() const
{
	XTHROW( x::bad_value_access, !is_int64(), "" );

	return to_ref().i64;
}

x::uint8 x::value::to_uint8() const
{
	XTHROW( x::bad_value_access, !is_uint8(), "" );

	return to_ref().u8;
}

x::uint16 x::value::to_uint16() const
{
	XTHROW( x::bad_value_access, !is_uint16(), "" );

	return to_ref().u16;
}

x::uint32 x::value::to_uint32() const
{
	XTHROW( x::bad_value_access, !is_uint32(), "" );

	return to_ref().u32;
}

x::uint64 x::value::to_uint64() const
{
	XTHROW( x::bad_value_access, !is_uint64(), "" );

	return to_ref().u64;
}

x::float16 x::value::to_float16() const
{
	XTHROW( x::bad_value_access, !is_float16(), "" );

	return to_ref().f16;
}

x::float32 x::value::to_float32() const
{
	XTHROW( x::bad_value_access, !is_float32(), "" );

	return to_ref().f32;
}

x::float64 x::value::to_float64() const
{
	XTHROW( x::bad_value_access, !is_float64(), "" );

	return to_ref().f64;
}

x::intptr x::value::to_intptr() const
{
	XTHROW( x::bad_value_access, !is_intptr(), "" );

	return to_ref().ptr;
}

x::string x::value::to_string() const
{
	XTHROW( x::bad_value_access, !is_string(), "" );

	return to_ref().str;
}

x::object * x::value::to_object() const
{
	XTHROW( x::bad_value_access, !is_object(), "" );

	return to_ref().obj;
}

x::value * x::value::to_reference() const
{
	return ref;
}

x::value & x::value::to_ref()
{
	x::value * result = this;

	while ( 1 )
	{
		if ( result->is_ref() )
			result = &result->to_ref();
		else
			break;
	}

	return *result;
}

const x::value & x::value::to_ref() const
{
	const x::value * result = this;

	while ( 1 )
	{
		if ( result->is_ref() )
			result = &result->to_ref();
		else
			break;
	}

	return *result;
}

x::value_handle::value_handle()
{
}

x::value_handle::value_handle( std::nullptr_t )
{
}

x::value_handle::value_handle( const x::value & val )
	: _value( val )
{
	attach_to_root();
}

x::value_handle::value_handle( const value_handle & val )
	: _value( val._value )
{
	attach_to_root();
}

x::value_handle::~value_handle()
{
	detach_to_root();
}

x::value_handle & x::value_handle::operator=( std::nullptr_t )
{
	detach_to_root();
	_value = x::value{};

	return *this;
}

x::value_handle & x::value_handle::operator=( const x::value & val )
{
	detach_to_root();
	_value = val;
	attach_to_root();

	return *this;
}

x::value_handle & x::value_handle::operator=( const value_handle & val )
{
	detach_to_root();
	_value = val._value;
	attach_to_root();

	return *this;
}

x::value_handle::operator bool() const
{
	return !_value.empty();
}

x::value & x::value_handle::operator*()
{
	return _value;
}

const x::value & x::value_handle::operator*() const
{
	return _value;
}

x::value * x::value_handle::operator->()
{
	return &_value;
}

const x::value * x::value_handle::operator->() const
{
	return &_value;
}

void x::value_handle::attach_to_root()
{
}

void x::value_handle::detach_to_root()
{
}
