#pragma once

#include "type.h"

#include <variant>

namespace x
{
	class value
	{
	public:
		value();
		value( const value & val );
		value & operator=( const value & val );
		~value();

	public:
		value( bool val );
		value( x::int8 val );
		value( x::int16 val );
		value( x::int32 val );
		value( x::int64 val );
		value( x::uint8 val );
		value( x::uint16 val );
		value( x::uint32 val );
		value( x::uint64 val );
		value( x::float16 val );
		value( x::float32 val );
		value( x::float64 val );
		value( x::string val );
		value( x::intptr val );
		value( x::object * val );
		value( x::value * val );
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0> value( T val )
		{
			i64 = (x::int64)val;
			_flags = x::value_t::INT64;
			_flags |= x::value_t::ENUM_MASK;
		}

	public:
		value & operator=( bool val );
		value & operator=( x::int8 val );
		value & operator=( x::int16 val );
		value & operator=( x::int32 val );
		value & operator=( x::int64 val );
		value & operator=( x::uint8 val );
		value & operator=( x::uint16 val );
		value & operator=( x::uint32 val );
		value & operator=( x::uint64 val );
		value & operator=( x::float16 val );
		value & operator=( x::float32 val );
		value & operator=( x::float64 val );
		value & operator=( x::string val );
		value & operator=( x::intptr val );
		value & operator=( x::object * val );
		value & operator=( x::value * val );
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0> value & operator=( T val )
		{
			i64 = (x::int64)val;
			_flags = x::value_t::INT64;
			_flags |= x::value_t::ENUM_MASK;
			return *this;
		}

	public:
		x::value_t type() const;

	public:
		bool is_ref() const;
		bool is_enum() const;
		bool is_async() const;
		bool is_invalid() const;

	public:
		bool is_null() const;
		bool is_bool() const;
		bool is_int8() const;
		bool is_int16() const;
		bool is_int32() const;
		bool is_int64() const;
		bool is_signed() const;
		bool is_uint8() const;
		bool is_uint16() const;
		bool is_uint32() const;
		bool is_uint64() const;
		bool is_unsigned() const;
		bool is_float16() const;
		bool is_float32() const;
		bool is_float64() const;
		bool is_floating() const;
		bool is_string() const;
		bool is_intptr() const;
		bool is_object() const;

	public:
		bool to_bool() const;
		x::int8 to_int8() const;
		x::int16 to_int16() const;
		x::int32 to_int32() const;
		x::int64 to_int64() const;
		x::uint8 to_uint8() const;
		x::uint16 to_uint16() const;
		x::uint32 to_uint32() const;
		x::uint64 to_uint64() const;
		x::float16 to_float16() const;
		x::float32 to_float32() const;
		x::float64 to_float64() const;
		x::string to_string() const;
		x::intptr to_intptr() const;
		x::object * to_object() const;
		x::value & to_reference();
		const x::value & to_reference() const;

	private:
		x::value_flags _flags;
		union
		{
			bool b;
			x::int8 i8;
			x::int16 i16;
			x::int32 i32;
			x::int64 i64;
			x::uint8 u8;
			x::uint16 u16;
			x::uint32 u32;
			x::uint64 u64;
			x::float16 f16;
			x::float32 f32;
			x::float64 f64;
			x::string str;
			x::intptr ptr;
			x::object * obj;
			x::value * ref;
		};
	};
}