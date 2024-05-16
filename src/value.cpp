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
	if ( to_reference()._flags == x::value_t::INVALID )
		return x::value_t::INVALID;
	else if ( to_reference()._flags == x::value_t::NIL )
		return x::value_t::NIL;
	else
		return ( to_reference()._flags & x::value_t::TYPE_MASK ).value();
}

x::meta_type_ptr x::value::meta() const
{
	return nullptr;
}

bool x::value::is_ref() const
{
	return _flags && x::value_t::REF_MASK;
}

bool x::value::is_enum() const
{
	return to_reference()._flags && x::value_t::ENUM_MASK;
}

bool x::value::is_flag() const
{
	return to_reference()._flags && x::value_t::FLAG_MASK;
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
	ASSERT( is_bool(), "" );

	return to_reference().b;
}

x::int8 x::value::to_int8() const
{
	ASSERT( is_int8(), "" );

	return to_reference().i8;
}

x::int16 x::value::to_int16() const
{
	ASSERT( is_int16(), "" );

	return to_reference().i16;
}

x::int32 x::value::to_int32() const
{
	ASSERT( is_int32(), "" );

	return to_reference().i32;
}

x::int64 x::value::to_int64() const
{
	ASSERT( is_int64(), "" );

	return to_reference().i64;
}

x::uint8 x::value::to_uint8() const
{
	ASSERT( is_uint8(), "" );

	return to_reference().u8;
}

x::uint16 x::value::to_uint16() const
{
	ASSERT( is_uint16(), "" );

	return to_reference().u16;
}

x::uint32 x::value::to_uint32() const
{
	ASSERT( is_uint32(), "" );

	return to_reference().u32;
}

x::uint64 x::value::to_uint64() const
{
	ASSERT( is_uint64(), "" );

	return to_reference().u64;
}

x::float16 x::value::to_float16() const
{
	ASSERT( is_float16(), "" );

	return to_reference().f16;
}

x::float32 x::value::to_float32() const
{
	ASSERT( is_float32(), "" );

	return to_reference().f32;
}

x::float64 x::value::to_float64() const
{
	ASSERT( is_float64(), "" );

	return to_reference().f64;
}

x::intptr x::value::to_intptr() const
{
	ASSERT( is_intptr(), "" );

	return to_reference().ptr;
}

x::string x::value::to_string() const
{
	ASSERT( is_string(), "" );

	return to_reference().str;
}

x::object * x::value::to_object() const
{
	ASSERT( is_object(), "" );

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
