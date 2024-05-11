#include "value.h"

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

x::value_t x::value::type() const
{
	if ( _flags == x::value_t::INVALID )
		return x::value_t::INVALID;
	else if ( _flags == x::value_t::NIL )
		return x::value_t::NIL;
	else
		return ( _flags & x::value_t::TYPE_MASK ).value();
}

x::meta_type_ptr x::value::meta() const
{
	return _meta;
}

bool x::value::is_ref() const
{
	return _flags && x::value_t::REF_MASK;
}

bool x::value::is_enum() const
{
	return _flags && x::value_t::ENUM_MASK;
}

bool x::value::is_flag() const
{
	return _flags && x::value_t::FLAG_MASK;
}

bool x::value::is_async() const
{
	return _flags && x::value_t::ASYN_MASK;
}

bool x::value::is_invalid() const
{
	return _flags == x::value_t::INVALID;
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
	return _flags || x::value_t::SIGNED_MASK;
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
	return _flags || x::value_t::UNSIGNED_MASK;
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
	return _flags || x::value_t::FLOATING_MASK;
}

bool x::value::is_string() const
{
	return type() == x::value_t::STRING;
}

bool x::value::is_object() const
{
	return type() == x::value_t::OBJECT;
}

bool x::value::is_intptr() const
{
	return type() == x::value_t::INTPTR;
}

bool x::value::to_bool() const
{
	ASSERT( is_bool(), "" );

	return b;
}

x::int8 x::value::to_int8() const
{
	ASSERT( is_int8(), "" );

	return i8;
}

x::int16 x::value::to_int16() const
{
	ASSERT( is_int16(), "" );

	return i16;
}

x::int32 x::value::to_int32() const
{
	ASSERT( is_int32(), "" );

	return i32;
}

x::int64 x::value::to_int64() const
{
	ASSERT( is_int64(), "" );

	return i64;
}

x::uint8 x::value::to_uint8() const
{
	ASSERT( is_uint8(), "" );

	return u8;
}

x::uint16 x::value::to_uint16() const
{
	ASSERT( is_uint16(), "" );

	return u16;
}

x::uint32 x::value::to_uint32() const
{
	ASSERT( is_uint32(), "" );

	return u32;
}

x::uint64 x::value::to_uint64() const
{
	ASSERT( is_uint64(), "" );

	return u64;
}

x::float16 x::value::to_float16() const
{
	ASSERT( is_float16(), "" );

	return f16;
}

x::float32 x::value::to_float32() const
{
	ASSERT( is_float32(), "" );

	return f32;
}

x::float64 x::value::to_float64() const
{
	ASSERT( is_float64(), "" );

	return f64;
}

x::object * x::value::to_object() const
{
	ASSERT( is_object(), "" );

	return obj;
}

x::intptr x::value::to_intptr() const
{
	ASSERT( is_intptr(), "" );

	return ptr;
}

const char * x::value::to_string() const
{
	ASSERT( is_string(), "" );

	return str;
}
