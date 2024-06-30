#pragma once

#include "type.h"

namespace x
{
	class buffer
	{
	public:
		buffer();

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
		uint64 read( x::value & val );
		uint64 write( const x::value & val );

	private:
		x::byte * get( x::uint64 size );
		x::byte * put( x::uint64 size );

	private:
		x::uint64 _ppos = 0;
		x::uint64 _gpos = 0;
		std::vector<x::byte> _data;
	};
}