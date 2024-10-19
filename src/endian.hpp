#pragma once

#include <bit>

namespace x
{
	class endian
	{
	public:
		template<typename T> inline static T swap_endian( T val )
		{
			static_assert ( CHAR_BIT == 8, "CHAR_BIT != 8" );

			union
			{
				T u;
				std::uint8_t u8[sizeof( T )];
			} src, dst;

			src.u = u;

			if constexpr ( sizeof( T ) == 2 )
			{
				dst.u8[0] = src.u8[1];
				dst.u8[1] = src.u8[0];
			}
			else if constexpr ( sizeof( T ) == 4 )
			{
				dst.u8[0] = src.u8[3];
				dst.u8[1] = src.u8[2];
				dst.u8[2] = src.u8[1];
				dst.u8[3] = src.u8[0];
			}
			else if constexpr ( sizeof( T ) == 8 )
			{
				dst.u8[0] = src.u8[7];
				dst.u8[1] = src.u8[6];
				dst.u8[2] = src.u8[5];
				dst.u8[3] = src.u8[4];
				dst.u8[4] = src.u8[3];
				dst.u8[5] = src.u8[2];
				dst.u8[6] = src.u8[1];
				dst.u8[7] = src.u8[0];
			}

			return dst.u;
		}

		template<typename T> inline static T native_to_big( T val )
		{
			if constexpr ( std::endian::native == std::endian::big )
				return val;
			else
				return swap_endian( val );
		}

		template<typename T> inline static T native_to_little( T val )
		{
			if constexpr ( std::endian::native == std::endian::little )
				return val;
			else
				return swap_endian( val );
		}

		template<typename T> inline static T big_to_native( T val )
		{
			if constexpr ( std::endian::native == std::endian::big )
				return val;
			else
				return swap_endian( val );
		}

		template<typename T> inline static T little_to_native( T val )
		{
			if constexpr ( std::endian::native == std::endian::little )
				return val;
			else
				return swap_endian( val );
		}
	};
}
