#pragma once

#include "type.h"

namespace x
{
	class buffer
	{
	public:
		buffer() = default;

	public:
		void resize( x::uint64 size );

	public:
		x::uint64 seekp() const;
		x::uint64 seekg() const;
		void tellp( x::uint64 i );
		void tellg( x::uint64 i );

	public:
		x::uint64 size() const;

	public:
		x::byte * data();
		const x::byte * data() const;

	public:
		x::uint64 read( x::value & val );
		x::uint64 write( const x::value & val );

	public:
		x::uint64 read( x::byte * data, x::uint64 size );
		x::uint64 write( const x::byte * data, x::uint64 size );

	private:
		x::byte * get( x::uint64 size );
		x::byte * put( x::uint64 size );

	private:
		x::uint64 _ppos = 0;
		x::uint64 _gpos = 0;
		std::vector<x::byte> _data;
	};

    class stream_buffer : public std::basic_streambuf<char, std::char_traits<char>>
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
        stream_buffer();
        stream_buffer( stream_buffer && _Right ) noexcept;
        stream_buffer( std::ios_base::openmode _Mode );
        stream_buffer( const buf_type & _Str, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out );
        template <class _Alloc2> stream_buffer( const std::vector<element_type, _Alloc2> & _Str, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out )
        {
            _Init( _Str.c_str(), _Str.size(), _Getstate( _Mode ) );
        }
        stream_buffer & operator=( stream_buffer && _Right ) noexcept;
        ~stream_buffer() noexcept override;

    private:
        stream_buffer( const stream_buffer & ) = delete;
        stream_buffer & operator=( const stream_buffer & ) = delete;

    public:
        void swap( stream_buffer & _Right ) noexcept;

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
        void _Assign_rv( stream_buffer && _Right ) noexcept;
        void _Tidy() noexcept;
        static int _Getstate( std::ios_base::openmode _Mode ) noexcept;

    private:
        int _Mystate = 0;
        buf_type _Data = {};
        buf_type _Prepare = {};
        element_type * _Seekhigh = nullptr;
    };
}