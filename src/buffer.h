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

	class stream_buffer : public std::streambuf
	{
	public:
		using traits = std::char_traits<char>;

	public:
		stream_buffer() = default;

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

	public:
		x::byte * prepare( x::uint64 size );
		void commit( x::uint64 size );
		void consume();

	private:
		x::byte * get( x::uint64 size );
		x::byte * put( x::uint64 size );

    protected:
        int_type overflow( int_type _Meta = traits::eof() ) override;
        int_type pbackfail( int_type _Meta = traits::eof() ) override;
        int_type underflow() override;
		pos_type seekoff( off_type _Off, std::ios_base::seekdir _Way, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out ) override;
		pos_type seekpos( pos_type _Pos, std::ios_base::openmode _Mode = std::ios_base::in | std::ios_base::out ) override;

	private:
		x::uint64 _ppos = 0;
		x::uint64 _gpos = 0;
		std::vector<x::byte> _data;
	};
}