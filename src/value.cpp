#include "value.h"

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

x::value::value( x::object * val )
{
	obj = val;
	_flags = x::value_t::OBJECT;
}

x::value::value( x::value * val )
{
	ref = val;
	_flags = x::value_t::REF_MASK;
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

x::value & x::value::operator=( x::object * val )
{
	obj = val;
	_flags = x::value_t::OBJECT;
	return *this;
}

x::value & x::value::operator=( x::value * val )
{
	ref = val;
	_flags = x::value_t::REF_MASK;
	return *this;
}

x::value_t x::value::type() const
{
	if ( to_reference()._flags == x::value_t::INVALID )
		return x::value_t::INVALID;
	else if ( to_reference()._flags == x::value_t::NIL )
		return x::value_t::NIL;
	else
		return ( to_reference()._flags & x::value_t::TYPE_MASK ).value();
}

bool x::value::is_ref() const
{
	return _flags && x::value_t::REF_MASK;
}

bool x::value::is_enum() const
{
	return to_reference()._flags && x::value_t::ENUM_MASK;
}

bool x::value::is_async() const
{
	return to_reference()._flags && x::value_t::ASYN_MASK;
}

bool x::value::is_invalid() const
{
	return to_reference()._flags == x::value_t::INVALID;
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
	return to_reference()._flags || x::value_t::SIGNED_MASK;
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
	return to_reference()._flags || x::value_t::UNSIGNED_MASK;
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
	return to_reference()._flags || x::value_t::FLOATING_MASK;
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
	XTHROW( x::bad_value_access, is_bool(), "" );

	return to_reference().b;
}

x::int8 x::value::to_int8() const
{
	XTHROW( x::bad_value_access, is_int8(), "" );

	return to_reference().i8;
}

x::int16 x::value::to_int16() const
{
	XTHROW( x::bad_value_access, is_int16(), "" );

	return to_reference().i16;
}

x::int32 x::value::to_int32() const
{
	XTHROW( x::bad_value_access, is_int32(), "" );

	return to_reference().i32;
}

x::int64 x::value::to_int64() const
{
	XTHROW( x::bad_value_access, is_int64(), "" );

	return to_reference().i64;
}

x::uint8 x::value::to_uint8() const
{
	XTHROW( x::bad_value_access, is_uint8(), "" );

	return to_reference().u8;
}

x::uint16 x::value::to_uint16() const
{
	XTHROW( x::bad_value_access, is_uint16(), "" );

	return to_reference().u16;
}

x::uint32 x::value::to_uint32() const
{
	XTHROW( x::bad_value_access, is_uint32(), "" );

	return to_reference().u32;
}

x::uint64 x::value::to_uint64() const
{
	XTHROW( x::bad_value_access, is_uint64(), "" );

	return to_reference().u64;
}

x::float16 x::value::to_float16() const
{
	XTHROW( x::bad_value_access, is_float16(), "" );

	return to_reference().f16;
}

x::float32 x::value::to_float32() const
{
	XTHROW( x::bad_value_access, is_float32(), "" );

	return to_reference().f32;
}

x::float64 x::value::to_float64() const
{
	XTHROW( x::bad_value_access, is_float64(), "" );

	return to_reference().f64;
}

x::intptr x::value::to_intptr() const
{
	XTHROW( x::bad_value_access, is_intptr(), "" );

	return to_reference().ptr;
}

x::string x::value::to_string() const
{
	XTHROW( x::bad_value_access, is_string(), "" );

	return to_reference().str;
}

x::object * x::value::to_object() const
{
	XTHROW( x::bad_value_access, is_object(), "" );

	return to_reference().obj;
}

x::value & x::value::to_reference()
{
	if ( is_ref() )
		return ref->to_reference();
	else
		return *this;
}

const x::value & x::value::to_reference() const
{
	if ( is_ref() )
		return ref->to_reference();
	else
		return *this;
}
