#pragma once

#include <exception>

namespace x
{
    template <class _Elem, class _Traits = std::char_traits<_Elem>> class static_basic_string_view
    {
    public:
        static_assert( std::is_same_v<_Elem, typename _Traits::char_type>,
                       "Bad char_traits for static_basic_string_view; N4950 [string.view.template.general]/1 "
                       "\"The program is ill-formed if traits::char_type is not the same type as charT.\"" );

        static_assert( !std::is_array_v<_Elem> && std::is_trivial_v<_Elem> && std::is_standard_layout_v<_Elem>,
                       "The character type of static_basic_string_view must be a non-array trivial standard-layout type. See N4950 "
                       "[strings.general]/1." );

    public:
        using std_string_view = std::basic_string_view<_Elem, _Traits>;
        using traits_type = _Traits;
        using value_type = _Elem;
        using pointer = _Elem *;
        using const_pointer = const _Elem *;
        using reference = _Elem &;
        using const_reference = const _Elem &;
        using const_iterator = std_string_view::const_iterator;
        using iterator = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator = const_reverse_iterator;
        using size_type = std::size_t;
        using difference_type = ptrdiff_t;

    public:
        static constexpr auto npos{ static_cast<size_type>( -1 ) };

    public:
        constexpr static_basic_string_view() noexcept : _Mystr(nullptr), _Mystart( 0 ), _Mysize( 0 )
        {
        }
        constexpr static_basic_string_view( const static_basic_string_view & ) noexcept = default;
        constexpr static_basic_string_view & operator=( const static_basic_string_view & ) noexcept = default;
        constexpr static_basic_string_view( std::string * _Str, const size_type _Start, const size_type _Count ) noexcept
            : _Mystr( _Str ), _Mystart( _Start ), _Mysize( _Count )
        {
        }

    private:
        constexpr std::basic_string<_Elem, _Traits> operator+( const static_basic_string_view<_Elem, _Traits> _Rhs ) const
        {
            std::basic_string<_Elem, _Traits> str;
            str.assign( data(), size() );
            str.append( _Rhs.data(), _Rhs.size() );
            return str;
        }

        constexpr std::basic_string<_Elem, _Traits> operator+( const std::basic_string_view<_Elem, _Traits> _Rhs ) const
        {
            std::basic_string<_Elem, _Traits> str;
            str.assign( data(), size() );
            str.append( _Rhs.data(), _Rhs.size() );
            return str;
        }

    public:
        constexpr const_iterator begin() const noexcept
        {
            return const_iterator( _Mydata(), _Mysize, 0 );
        }

        constexpr const_iterator end() const noexcept
        {
            return const_iterator( _Mydata(), _Mysize, _Mysize );
        }

        constexpr const_iterator cbegin() const noexcept
        {
            return begin();
        }

        constexpr const_iterator cend() const noexcept
        {
            return end();
        }

        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{ end() };
        }

        constexpr const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{ begin() };
        }

        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return rbegin();
        }

        constexpr const_reverse_iterator crend() const noexcept
        {
            return rend();
        }

        constexpr const_pointer _Unchecked_begin() const noexcept
        {
            return _Mydata();
        }

        constexpr const_pointer _Unchecked_end() const noexcept
        {
            return _Mydata() + _Mysize;
        }

        constexpr size_type start() const
        {
            return _Mystart;
        }

        constexpr size_type size() const noexcept
        {
            return _Mysize;
        }

        constexpr size_type length() const noexcept
        {
            return _Mysize;
        }

        constexpr bool empty() const noexcept
        {
            return _Mysize == 0;
        }

        constexpr const_pointer data() const noexcept
        {
            return _Mydata();
        }

        constexpr size_type max_size() const noexcept
        {
            return ( _STD min )( static_cast<std::size_t>( PTRDIFF_MAX ), static_cast<std::size_t>( -1 ) / sizeof( _Elem ) );
        }

        constexpr const_reference operator[]( const size_type _Off ) const noexcept
        {
            return _Mydata()[_Off];
        }

        constexpr const_reference at( const size_type _Off ) const
        {
            _Check_offset_exclusive( _Off );
            return _Mydata()[_Off];
        }

        constexpr const_reference front() const noexcept
        {
            return _Mydata()[0];
        }

        constexpr const_reference back() const noexcept
        {
            return _Mydata()[_Mysize - 1];
        }

        constexpr void remove_prefix( const size_type _Count ) noexcept
        {
            _Mystart += _Count;
            _Mysize -= _Count;
        }

        constexpr void remove_suffix( const size_type _Count ) noexcept
        {
            _Mysize -= _Count;
        }

        constexpr void swap( static_basic_string_view & _Other ) noexcept
        {
            const static_basic_string_view _Tmp{ _Other };
            _Other = *this;
            *this = _Tmp;
        }

        size_type copy( _Elem * const _Ptr, size_type _Count, const size_type _Off = 0 ) const
        {
            _Check_offset( _Off );
            _Count = _Clamp_suffix_size( _Off, _Count );
            _Traits::copy( _Ptr, _Mydata() + _Off, _Count );
            return _Count;
        }

        size_type _Copy_s( _Elem * const _Dest, const size_type _Dest_size, size_type _Count, const size_type _Off = 0 ) const
        {
            _Check_offset( _Off );
            _Count = _Clamp_suffix_size( _Off, _Count );
            _Traits::_Copy_s( _Dest, _Dest_size, _Mydata() + _Off, _Count );
            return _Count;
        }

        constexpr static_basic_string_view substr( const size_type _Off = 0, size_type _Count = npos ) const
        {
            _Check_offset( _Off );
            _Count = _Clamp_suffix_size( _Off, _Count );
            return static_basic_string_view( _Mystr, _Mystart + _Off, _Count );
        }

        constexpr bool _Equal( const static_basic_string_view _Right ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize )._Equal( { _Right._Mydata(), _Right._Mysize } );
        }

        constexpr int compare( const static_basic_string_view _Right ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).compare( { _Right._Mydata(), _Right._Mysize } );
        }

        constexpr int compare( const size_type _Off, const size_type _Nx, const static_basic_string_view _Right ) const
        {
            return substr( _Off, _Nx ).compare( _Right );
        }

        constexpr int compare( const size_type _Off, const size_type _Nx, const static_basic_string_view _Right, const size_type _Roff, const size_type _Count ) const
        {
            return substr( _Off, _Nx ).compare( _Right.substr( _Roff, _Count ) );
        }

        constexpr int compare( const _Elem * const _Ptr ) const noexcept
        {
            return compare( static_basic_string_view( _Ptr ) );
        }

        constexpr int compare( const size_type _Off, const size_type _Nx, const _Elem * const _Ptr ) const
        {
            return substr( _Off, _Nx ).compare( static_basic_string_view( _Ptr ) );
        }

        constexpr int compare( const size_type _Off, const size_type _Nx, const _Elem * const _Ptr, const size_type _Count ) const
        {
            return substr( _Off, _Nx ).compare( static_basic_string_view( _Ptr, _Count ) );
        }

        constexpr bool starts_with( const static_basic_string_view _Right ) const noexcept
        {
            const auto _Rightsize = _Right._Mysize;
            if ( _Mysize < _Rightsize )
            {
                return false;
            }
            return _Traits::compare( _Mydata(), _Right._Mydata(), _Rightsize ) == 0;
        }

        constexpr bool starts_with( const _Elem _Right ) const noexcept
        {
            return !empty() && _Traits::eq( front(), _Right );
        }

        constexpr bool starts_with( const _Elem * const _Right ) const noexcept
        {
            return starts_with( static_basic_string_view( _Right ) );
        }

        constexpr bool ends_with( const static_basic_string_view _Right ) const noexcept
        {
            const auto _Rightsize = _Right._Mysize;
            if ( _Mysize < _Rightsize )
            {
                return false;
            }
            return _Traits::compare( _Mydata() + ( _Mysize - _Rightsize ), _Right._Mydata(), _Rightsize ) == 0;
        }

        constexpr bool ends_with( const _Elem _Right ) const noexcept
        {
            return !empty() && _Traits::eq( back(), _Right );
        }

        constexpr bool ends_with( const _Elem * const _Right ) const noexcept
        {
            return ends_with( static_basic_string_view( _Right ) );
        }

        constexpr bool contains( const static_basic_string_view _Right ) const noexcept
        {
            return find( _Right ) != npos;
        }

        constexpr bool contains( const _Elem _Right ) const noexcept
        {
            return find( _Right ) != npos;
        }

        constexpr bool contains( const _Elem * const _Right ) const noexcept
        {
            return find( _Right ) != npos;
        }

        constexpr size_type find( const static_basic_string_view _Right, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type find( const _Elem _Ch, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find( _Ch, _Off );
        }

        constexpr size_type find( const _Elem * const _Ptr, const size_type _Off, const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find( _Ptr, _Off, _Count );
        }

        constexpr size_type find( const _Elem * const _Ptr, const size_type _Off = 0 ) const noexcept  
        {
            return std_string_view( _Mydata(), _Mysize ).find( _Ptr, _Off );
        }

        constexpr size_type rfind( const static_basic_string_view _Right, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).rfind( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type rfind( const _Elem _Ch, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).rfind( _Ch, _Off );
        }

        constexpr size_type rfind( const _Elem * const _Ptr, const size_type _Off, const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).rfind( _Ptr, _Off, _Count );
        }

        constexpr size_type rfind( const _Elem * const _Ptr, const size_type _Off = npos ) const noexcept  
        {
            return std_string_view( _Mydata(), _Mysize ).rfind( _Ptr, _Off );
        }

        constexpr size_type find_first_of( const static_basic_string_view _Right, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_of( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type find_first_of( const _Elem _Ch, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_of( _Ch, _Off );
        }

        constexpr size_type find_first_of( const _Elem * const _Ptr, const size_type _Off, const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_of( _Ptr, _Off, _Count );
        }

        constexpr size_type find_first_of( const _Elem * const _Ptr, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_of( _Ptr, _Off );
        }

        constexpr size_type find_last_of( const static_basic_string_view _Right, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_of( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type find_last_of( const _Elem _Ch, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_of( _Ch, _Off );
        }

        constexpr size_type find_last_of( const _Elem * const _Ptr, const size_type _Off,const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_of( _Ptr, _Off, _Count );
        }

        constexpr size_type find_last_of( const _Elem * const _Ptr, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_of( _Ptr, _Off );
        }

        constexpr size_type find_first_not_of( const static_basic_string_view _Right, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_not_of( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type find_first_not_of( const _Elem _Ch, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_not_of( _Ch, _Off );
        }

        constexpr size_type find_first_not_of( const _Elem * const _Ptr, const size_type _Off, const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_not_of( _Ptr, _Off, _Count );
        }

        constexpr size_type find_first_not_of( const _Elem * const _Ptr, const size_type _Off = 0 ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_first_not_of( _Ptr, _Off );
        }

        constexpr size_type find_last_not_of( const static_basic_string_view _Right, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_not_of( { _Right._Mydata(), _Right._Mysize }, _Off );
        }

        constexpr size_type find_last_not_of( const _Elem _Ch, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_not_of( _Ch, _Off );
        }

        constexpr size_type find_last_not_of( const _Elem * const _Ptr, const size_type _Off, const size_type _Count ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_not_of( _Ptr, _Off, _Count );
        }

        constexpr size_type find_last_not_of( const _Elem * const _Ptr, const size_type _Off = npos ) const noexcept
        {
            return std_string_view( _Mydata(), _Mysize ).find_last_not_of( _Ptr, _Off );
        }

    private:
        constexpr bool _Starts_with( const static_basic_string_view _View ) const noexcept
        {
            return _Mysize >= _View._Mysize && _Traits::compare( _Mydata(), _View._Mydata(), _View._Mysize ) == 0;
        }

        constexpr void _Check_offset( const size_type _Off ) const
        {
            if ( _Mysize < _Off )
            {
                throw std::exception( "invalid string_view position" );
            }
        }

        constexpr void _Check_offset_exclusive( const size_type _Off ) const
        {
            if ( _Mysize <= _Off )
            {
                throw std::exception( "invalid string_view position" );
            }
        }

        constexpr size_type _Clamp_suffix_size( const size_type _Off, const size_type _Size ) const noexcept
        {
            return ( _STD min )( _Size, _Mysize - _Off );
        }

        const_pointer _Mydata() const
        {
            return _Mystr->c_str() + _Mystart;
        }

        std::string * _Mystr;
        size_type _Mystart;
        size_type _Mysize;
    };

    template <class _Elem, class _Traits> constexpr std::basic_string<_Elem, _Traits> operator+( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        std::basic_string<_Elem, _Traits> str;
        str.assign( _Lhs.data(), _Lhs.size() );
        str.append( _Rhs.data(), _Rhs.size() );
        return str;
    }

    template <class _Elem, class _Traits> constexpr std::basic_string<_Elem, _Traits> operator+( const static_basic_string_view<_Elem, _Traits> _Lhs, const std::basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        std::basic_string<_Elem, _Traits> str;
        str.assign( _Lhs.data(), _Lhs.size() );
        str.append( _Rhs.data(), _Rhs.size() );
        return str;
    }

    template <class _Elem, class _Traits> constexpr std::basic_string<_Elem, _Traits> operator+( const std::basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        std::basic_string<_Elem, _Traits> str;
        str.assign( _Lhs.data(), _Lhs.size() );
        str.append( _Rhs.data(), _Rhs.size() );
        return str;
    }

    template <class _Elem, class _Traits> constexpr std::basic_string<_Elem, _Traits> operator+( const std::basic_string<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        std::basic_string<_Elem, _Traits> str;
        str.assign( _Lhs.data(), _Lhs.size() );
        str.append( _Rhs.data(), _Rhs.size() );
        return str;
    }

    template <class _Elem, class _Traits> constexpr std::basic_string<_Elem, _Traits> operator+( const std::basic_string<_Elem, _Traits> _Lhs, const std::basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        std::basic_string<_Elem, _Traits> str;
        str.assign( _Lhs.data(), _Lhs.size() );
        str.append( _Rhs.data(), _Rhs.size() );
        return str;
    }

    template <class _Elem, class _Traits> constexpr bool operator==( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return _Lhs._Equal( _Rhs );
    }

    template <class _Elem, class _Traits> constexpr bool operator!=( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return !_Lhs._Equal( _Rhs );
    }

    template <class _Elem, class _Traits> constexpr bool operator<( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return _Lhs.compare( _Rhs ) < 0;
    }

    template <class _Elem, class _Traits> constexpr bool operator>( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return _Lhs.compare( _Rhs ) > 0;
    }

    template <class _Elem, class _Traits> constexpr bool operator<=( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return _Lhs.compare( _Rhs ) <= 0;
    }

    template <class _Elem, class _Traits> constexpr bool operator>=( const static_basic_string_view<_Elem, _Traits> _Lhs, const static_basic_string_view<_Elem, _Traits> _Rhs ) noexcept
    {
        return _Lhs.compare( _Rhs ) >= 0;
    }

    using static_string_view = static_basic_string_view<char>;
}