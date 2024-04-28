#pragma once

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
