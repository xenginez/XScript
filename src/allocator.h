#pragma once

#include "type.h"

namespace x
{
	struct segment
	{

	};
	struct block
	{

	};
	struct page
	{

	};
	struct tld
	{

	};

	class allocator
	{
	private:
		struct private_p;

	public:
		allocator();
		~allocator();

	public:
		void * alloc( x::uint64 size );
		void free(void * ptr, x::uint64 size );

	public:


	public:
		void * valloc( x::uint64 size, vallocflag_t flag );
		void vfree( void * ptr, x::uint64 size );

	private:
		private_p * _p;
	};
}
