#pragma once

#include <limits>
#include <compare>

namespace x
{
	class float16
	{
	public:
		float16();
		float16( float val );
		float16( const float16 & val );
		float16 & operator=( float val );
		float16 & operator=( const float16 & val );

	public:
		operator float() const;

	public:
		float16 & operator+=( float other );
		float16 & operator-=( float other );
		float16 & operator*=( float other );
		float16 & operator/=( float other );
		float16 & operator+=( float16 other );
		float16 & operator-=( float16 other );
		float16 & operator*=( float16 other );
		float16 & operator/=( float16 other );

	public:
		float16 operator+( float other ) const;
		float16 operator-( float other ) const;
		float16 operator*( float other ) const;
		float16 operator/( float other ) const;
		float16 operator+( float16 other ) const;
		float16 operator-( float16 other ) const;
		float16 operator*( float16 other ) const;
		float16 operator/( float16 other ) const;

	public:
		std::strong_ordering operator<=>( float right ) const;
		std::strong_ordering operator<=>( float16 right ) const;

	public:
		float to_float() const;
		void from_float( float val );
		std::uint16_t to_uint16() const;
		void from_uint16( std::uint16_t val );

	private:
		union
		{
			struct
			{
				std::uint16_t mantissa : 10;
				std::uint16_t exponent : 5;
				std::uint16_t sign : 1;
			};

			std::uint16_t encode;
		};
	};
}

x::float16 operator+( float left, x::float16 right );
x::float16 operator-( float left, x::float16 right );
x::float16 operator*( float left, x::float16 right );
x::float16 operator/( float left, x::float16 right );
std::strong_ordering operator<=>( float left, x::float16 right );

namespace std
{
	template <> class numeric_limits<x::float16>
	{
	public:
		static constexpr bool has_infinity = true;
		static constexpr bool has_quiet_NaN = true;
		static constexpr bool has_signaling_NaN = true;
		static constexpr bool is_bounded = true;
		static constexpr bool is_iec559 = true;
		static constexpr bool is_signed = true;
		static constexpr bool is_specialized = true;
		static constexpr std::float_round_style round_style = std::float_round_style::round_to_nearest;
		static constexpr int radix = FLT_RADIX;

	public:
		static x::float16 min()
		{
			return -65504.0f;
		}

		static x::float16 max() noexcept
		{
			return 65504.0f;
		}

		static x::float16 lowest() noexcept
		{
			return -max();
		}

		static x::float16 epsilon() noexcept
		{
			return 5.9604645E-08F;
		}

		static x::float16 round_error() noexcept
		{
			return 0.5F;
		}

		static x::float16 denorm_min() noexcept
		{
			return FLT_TRUE_MIN;
		}

		static x::float16 infinity() noexcept
		{
			return __builtin_huge_valf();
		}

		static x::float16 quiet_NaN() noexcept
		{
			return __builtin_nanf( "0" );
		}

		static x::float16 signaling_NaN() noexcept
		{
			return __builtin_nansf( "1" );
		}

		static constexpr int digits = 11;
		static constexpr int digits10 = 3;
		static constexpr int max_digits10 = 6;
		static constexpr int max_exponent = 16;
		static constexpr int max_exponent10 = FLT_MAX_10_EXP;
		static constexpr int min_exponent = -13;
		static constexpr int min_exponent10 = FLT_MIN_10_EXP;
	};
}