#include "float16.h"

#include <bit>

namespace
{
	union big_float32
	{
		struct
		{
			std::uint16_t sign : 1;
			std::uint16_t exponent : 8;
			std::uint32_t mantissa : 23;
		};

		float encode;
	};
	union little_float32
	{
		struct
		{
			std::uint32_t mantissa : 23;
			std::uint16_t exponent : 8;
			std::uint16_t sign : 1;
		};

		float encode;
	};
}

x::float16::float16()
	:encode( 0 )
{

}

x::float16::float16( float val )
{
	from_float( val );
}

x::float16::float16( const x::float16 & val )
	:encode( val.encode )
{

}

x::float16 & x::float16::operator=( float val )
{
	from_float( val );

	return *this;
}

x::float16 & x::float16::operator=( const x::float16 & val )
{
	encode = val.encode;

	return *this;
}

x::float16::operator float() const
{
	return to_float();
}

x::float16 & x::float16::operator+=( float other )
{
	from_float( to_float() + other );

	return *this;
}

x::float16 & x::float16::operator-=( float other )
{
	from_float( to_float() - other );

	return *this;
}

x::float16 & x::float16::operator*=( float other )
{
	from_float( to_float() * other );

	return *this;
}

x::float16 & x::float16::operator/=( float other )
{
	from_float( to_float() / other );

	return *this;
}

x::float16 & x::float16::operator+=( float16 other )
{
	 return *this += other.to_float();
}

x::float16 & x::float16::operator-=( float16 other )
{
	return *this -= other.to_float();
}

x::float16 & x::float16::operator*=( float16 other )
{
	return *this *= other.to_float();
}

x::float16 & x::float16::operator/=( float16 other )
{
	return *this /= other.to_float();
}

x::float16 x::float16::operator+( float other ) const
{
	return to_float() + other;
}

x::float16 x::float16::operator-( float other ) const
{
	return to_float() - other;
}

x::float16 x::float16::operator*( float other ) const
{
	return to_float() * other;
}

x::float16 x::float16::operator/( float other ) const
{
	return to_float() / other;
}

x::float16 x::float16::operator+( float16 other ) const
{
	return *this + other.to_float();
}

x::float16 x::float16::operator-( float16 other ) const
{
	return *this - other.to_float();
}

x::float16 x::float16::operator*( float16 other ) const
{
	return *this * other.to_float();
}

x::float16 x::float16::operator/( float16 other ) const
{
	return *this / other.to_float();
}

std::strong_ordering x::float16::operator<=>( float right ) const
{
	float left = to_float();

	if ( ( std::abs( left ) - std::abs( right ) ) < std::numeric_limits<float>::epsilon() )
		return std::strong_ordering::equal;
	else if ( left < right )
		return std::strong_ordering::less;
	else
		return std::strong_ordering::greater;
}

std::strong_ordering x::float16::operator<=>( float16 right ) const
{
	return *this <=> right.to_float();
}

float x::float16::to_float() const
{
	if constexpr ( std::endian::native == std::endian::big )
	{
		big_float32 f{};
		f.sign = this->sign;
		f.exponent = this->exponent;
		f.mantissa = this->mantissa;
		return f.encode;
	}
	else
	{
		little_float32 f{};
		f.sign = this->sign;
		f.exponent = this->exponent;
		f.mantissa = this->mantissa;
		return f.encode;
	}
}

void x::float16::from_float( float val )
{
	if constexpr ( std::endian::native == std::endian::big )
	{
		big_float32 f{};
		f.encode = val;
		this->sign = f.sign;
		this->exponent = f.exponent;
		this->mantissa = f.mantissa;
	}
	else
	{
		little_float32 f{};
		f.encode = val;
		this->sign = f.sign;
		this->exponent = f.exponent;
		this->mantissa = f.mantissa;
	}
}

x::float16 operator+( float left, x::float16 right )
{
	return x::float16( left ) + right;
}

x::float16 operator-( float left, x::float16 right )
{
	return x::float16( left ) - right;
}

x::float16 operator*( float left, x::float16 right )
{
	return x::float16( left ) * right;
}

x::float16 operator/( float left, x::float16 right )
{
	return x::float16( left ) / right;
}

std::strong_ordering operator<=>( float left, x::float16 right )
{
	return x::float16( left ) <=> right;
}
