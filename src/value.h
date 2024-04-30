#pragma once

#include "type.h"

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
		x::value_t type() const;
		x::meta_type_ptr meta() const;

	public:
		bool is_ref() const;
		bool is_enum() const;
		bool is_flag() const;
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
		x::object * to_object() const;
		std::string_view to_string() const;

	private:
		x::value_flags _flags;
		x::meta_type_ptr _meta;
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
			x::object * obj;
			std::string_view str;
		};
	};
}