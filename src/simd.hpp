#pragma once

#include <immintrin.h>

namespace simd
{
	class int128
	{
	protected:
		union
		{
			struct
			{
				std::int8_t i8[16];
				std::int16_t i16[8];
				std::int32_t i32[4];
				std::int64_t i64[2];
			};
			__m128i i128;
		};

	protected:
		int128()
			: i128( _mm_setzero_si128() )
		{
		}
		int128( __m128i val )
			: i128( val )
		{
		}

	protected:
		static inline int128 set_i8( std::int8_t val )
		{
			return _mm_set1_epi8( val );
		}
		static inline int128 set_i8( std::int8_t * val )
		{
			return _mm_set_epi8( val[15], val[14], val[13], val[12], val[11], val[10], val[9], val[8], val[7], val[6], val[5], val[4], val[3], val[2], val[1], val[0] );
		}
		static inline int128 set_i16( std::int16_t val )
		{
			return _mm_set1_epi16( val );
		}
		static inline int128 set_i16( std::int16_t * val )
		{
			return _mm_set_epi16( val[7], val[6], val[5], val[4], val[3], val[2], val[1], val[0] );
		}
		static inline int128 set_i32( std::int32_t val )
		{
			return _mm_set1_epi32( val );
		}
		static inline int128 set_i32( std::int32_t * val )
		{
			return _mm_set_epi32( val[3], val[2], val[1], val[0] );
		}
		static inline int128 set_i64( std::int64_t val )
		{
			return _mm_set1_epi64x( val );
		}
		static inline int128 set_i64( std::int64_t * val )
		{
			return _mm_set_epi64x( val[1], val[0] );
		}

	protected:
		static inline int128 add_i8( int128 left, int128 right )
		{
			return _mm_add_epi8( left.i128, right.i128 );
		}
		static inline int128 add_i16( int128 left, int128 right )
		{
			return _mm_add_epi16( left.i128, right.i128 );
		}
		static inline int128 add_i32( int128 left, int128 right )
		{
			return _mm_add_epi32( left.i128, right.i128 );
		}
		static inline int128 add_i64( int128 left, int128 right )
		{
			return _mm_add_epi64( left.i128, right.i128 );
		}

		static inline int128 sub_i8( int128 left, int128 right )
		{
			return _mm_sub_epi8( left.i128, right.i128 );
		}
		static inline int128 sub_i16( int128 left, int128 right )
		{
			return _mm_sub_epi16( left.i128, right.i128 );
		}
		static inline int128 sub_i32( int128 left, int128 right )
		{
			return _mm_sub_epi32( left.i128, right.i128 );
		}
		static inline int128 sub_i64( int128 left, int128 right )
		{
			return _mm_sub_epi64( left.i128, right.i128 );
		}

		static inline int128 mul_i8( int128 left, int128 right );
		static inline int128 mul_i16( int128 left, int128 right );
		static inline int128 mul_i32( int128 left, int128 right )
		{
			return _mm_mullo_epi32( left.i128, right.i128 );
		}
		static inline int128 mul_i64( int128 left, int128 right )
		{	
			return _mm_mullo_epi64( left.i128, right.i128 );
		}

		static inline int128 div_i8( int128 left, int128 right )
		{
			return _mm_div_epi8( left.i128, right.i128 );
		}
		static inline int128 div_i16( int128 left, int128 right )
		{
			return _mm_div_epi16( left.i128, right.i128 );
		}
		static inline int128 div_i32( int128 left, int128 right )
		{
			return _mm_div_epi32( left.i128, right.i128 );
		}
		static inline int128 div_i64( int128 left, int128 right )
		{
			return _mm_div_epi64( left.i128, right.i128 );
		}
	};
	class int128_8x16 : public int128
	{

	};
	class int128_16x8 : public int128
	{

	};
	class int128_32x4 : public int128
	{

	};
	class int128_64x2 : public int128
	{

	};

	class uint128
	{
	public:
		union
		{
			struct
			{
				std::uint8_t u8[16];
				std::uint16_t u16[8];
				std::uint32_t u32[4];
				std::uint64_t u64[2];
			};
			__m128i u128;
		};
	};
	class uint128_8x16 : public uint128
	{

	};
	class uint128_16x8 : public uint128
	{

	};
	class uint128_32x4 : public uint128
	{

	};
	class uint128_64x2 : public uint128
	{

	};

	class float128
	{
	public:
		union
		{
			struct
			{
				float f32[4];
				double f64[2];
			};
			__m128d f128;
		};
	};
	class float128_32x4 : public float128
	{

	};
	class float128_64x2 : public float128
	{

	};

	class int256
	{
	public:
		union
		{
			struct
			{
				std::int8_t i8[32];
				std::int16_t i16[16];
				std::int32_t i32[8];
				std::int64_t i64[4];
				simd::int128 i128[2];
			};
			__m256i i256;
		};
	};
	class int256_8x32 : public int256
	{

	};
	class int256_16x16 : public int256
	{

	};
	class int256_32x8 : public int256
	{

	};
	class int256_64x4 : public int256
	{

	};
	class int256_128x2 : public int256
	{

	};

	class uint256
	{
	public:
		union
		{
			struct
			{
				std::uint8_t u8[32];
				std::uint16_t u16[16];
				std::uint32_t u32[8];
				std::uint64_t u64[4];
				simd::uint128 u128[2];
			};
			__m256i u256;
		};
	};
	class uint256_8x32 : public uint256
	{

	};
	class uint256_16x16 : public uint256
	{

	};
	class uint256_32x8 : public uint256
	{

	};
	class uint256_64x4 : public uint256
	{

	};
	class uint256_128x2 : public uint256
	{

	};

	class float256
	{
	public:
		union
		{
			struct
			{
				float f32[8];
				double f64[4];
				simd::float128 f128[2];
			};
			__m256d f256;
		};
	};
	class float256_32x8 : public float256
	{

	};
	class float256_64x4 : public float256
	{

	};
	class float256_128x2 : public float256
	{

	};

	class int512
	{
	public:
		union
		{
			struct
			{
				std::int8_t i8[64];
				std::int16_t i16[32];
				std::int32_t i32[16];
				std::int64_t i64[8];
				simd::int128 i128[4];
				simd::int256 i256[2];
			};
			__m512i i512;
		};
	};
	class int512_8x64 : public int512
	{

	};
	class int512_16x32 : public int512
	{

	};
	class int512_32x16 : public int512
	{

	};
	class int512_64x8 : public int512
	{

	};
	class int512_128x4 : public int512
	{

	};
	class int512_256x2 : public int512
	{

	};

	class uint512
	{
	public:
		union
		{
			struct
			{
				std::uint8_t u8[64];
				std::uint16_t u16[32];
				std::uint32_t u32[16];
				std::uint64_t u64[8];
				simd::uint128 u128[4];
				simd::uint256 u256[2];
			};
			__m512i u512;
		};
	};
	class uint512_8x64 : public uint512
	{

	};
	class uint512_16x32 : public uint512
	{

	};
	class uint512_32x16 : public uint512
	{

	};
	class uint512_64x8 : public uint512
	{

	};
	class uint512_128x4 : public uint512
	{

	};
	class uint512_256x2 : public uint512
	{

	};

	class float512
	{
	public:
		union
		{
			struct
			{
				float f32[16];
				double f64[8];
				simd::float128 f128[4];
				simd::float256 f256[2];
			};
			__m512d f512;
		};
	};
	class float512_32x16 : public float512
	{

	};
	class float512_64x8 : public float512
	{

	};
	class float512_128x4 : public float512
	{

	};
	class float512_256x2 : public float512
	{

	};
}
