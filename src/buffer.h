#pragma once

#include "type.h"

namespace x
{
    class buffer : public std::basic_streambuf<char, std::char_traits<char>>
    {
    public:
        using element_type = char;
        using buf_type = std::vector<element_type>;
        using traits_type = std::char_traits<element_type>;
        using streambuf_type = std::basic_streambuf<element_type, std::char_traits<element_type>>;
        
    public:
        using istreambuf_iterator = std::istreambuf_iterator<element_type>;

    public:
        using int_type = typename traits_type::int_type;
        using pos_type = typename traits_type::pos_type;
        using off_type = typename traits_type::off_type;
        using size_type = typename buf_type::size_type;

    public:
        buffer();
        buffer( buffer && _Right ) noexcept;
        buffer( std::ios_base::openmode _Mode );
        buffer( const buf_type & _Str, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out );
        template <class _Alloc2> buffer( const std::vector<element_type, _Alloc2> & _Str, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out )
        {
            _Init( _Str.c_str(), _Str.size(), _Getstate( _Mode ) );
        }
        buffer & operator=( buffer && _Right ) noexcept;
        ~buffer() noexcept override;

    private:
        buffer( const buffer & ) = delete;
        buffer & operator=( const buffer & ) = delete;

    public:
        void swap( buffer & _Right ) noexcept;

    public:
        element_type * prepare( size_type size );
        void commit( size_type size );
        void consume( size_type size );

    public:
        size_type size() const;
        const element_type * data() const;

    public:
        istreambuf_iterator begin();
        istreambuf_iterator end();

    protected:
        int_type overflow( int_type _Meta = traits_type::eof() ) override;
        int_type pbackfail( int_type _Meta = traits_type::eof() ) override;
        int_type underflow() override;
        pos_type seekoff( off_type _Off, std::ios_base::seekdir _Way, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out ) override;
        pos_type seekpos( pos_type _Pos, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out ) override;

    private:
        void _Init( const element_type * _Ptr, const size_type _Count, int _State );
        void _Assign_rv( buffer && _Right ) noexcept;
        void _Tidy() noexcept;
        static int _Getstate( std::ios_base::openmode _Mode ) noexcept;

    private:
        int _Mystate = 0;
        buf_type _Data = {};
        buf_type _Prepare = {};
        element_type * _Seekhigh = nullptr;
    };
}