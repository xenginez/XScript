#pragma once

namespace x
{
    template<typename T> class flags
    {
        static_assert( std::is_enum_v< T >, "must be an enum type" );

    public:
        using enum_type = T;

    public:
        flags()
            :_value( 0 )
        {

        }

        flags( enum_type val )
            :_value( static_cast<std::uint64_t>( val ) )
        {

        }

        flags( std::uint64_t val )
            :_value( val )
        {

        }

        flags( const flags & val )
            :_value( val._value )
        {

        }

        flags & operator=( enum_type val )
        {
            _value = static_cast<std::uint64_t>( val );
            return *this;
        }

        flags & operator=( std::uint64_t val )
        {
            _value = val;
            return *this;
        }

        flags & operator=( const flags & val )
        {
            _value = val._value;
            return *this;
        }

    public:
        operator bool() const
        {
            return _value != 0;
        }

    public:
        flags operator ~() const
        {
            return ~_value;
        }

    public:
        flags operator |( enum_type val ) const
        {
            return _value | static_cast<std::uint64_t>( val );
        }

        flags operator |( const flags & val ) const
        {
            return _value | val._value;
        }

        flags operator &( enum_type val ) const
        {
            return _value & static_cast<std::uint64_t>( val );
        }

        flags operator &( const flags & val ) const
        {
            return _value & val._value;
        }

        flags operator << ( std::uint64_t val ) const
        {
            return _value << val._value;
        }

        flags operator >> ( std::uint64_t val ) const
        {
            return _value >> val._value;
        }

    public:
        flags & operator |=( enum_type val )
        {
            _value |= static_cast<std::uint64_t>( val );

            return *this;
        }

        flags & operator |=( const flags & val )
        {
            _value |= val._value;

            return *this;
        }

        flags & operator &=( enum_type val )
        {
            _value &= static_cast<std::uint64_t>( val );

            return *this;
        }

        flags & operator &=( const flags & val )
        {
            _value &= val._value;

            return *this;
        }

        flags & operator <<=( std::uint64_t val )
        {
            _value <<= val._value;

            return *this;
        }

        flags & operator >>=( std::uint64_t val )
        {
            _value >>= val._value;

            return *this;
        }

    public:
        bool operator ||( enum_type val ) const
        {
            return ( _value | static_cast<std::uint64_t>( val ) ) != 0;
        }

        bool operator ||( const flags & val ) const
        {
            return ( _value | val._value ) != 0;
        }

        bool operator &&( enum_type val ) const
        {
            return ( _value & static_cast<std::uint64_t>( val ) ) != 0;
        }

        bool operator &&( const flags & val ) const
        {
            return ( _value & val._value ) != 0;
        }

        bool operator ==( enum_type val ) const
        {
            return _value == static_cast<std::uint64_t>( val );
        }

        bool operator ==( const flags & val ) const
        {
            return _value == val._value;
        }

        bool operator !=( enum_type val ) const
        {
            return _value != static_cast<std::uint64_t>( val );
        }

        bool operator !=( const flags & val ) const
        {
            return _value != val._value;
        }

    public:
        enum_type value() const
        {
            return static_cast<enum_type>( _value );
        }

    private:
        std::uint64_t _value;
    };
}