#pragma once

#include "type.h"
#include <memory_resource>

namespace x
{
	class allocator
	{
		friend class runtime;

	private:
		struct private_p;
		struct segment;
		struct block;
		struct page;
		struct heap;
		struct tld;

	public:
		allocator();
		~allocator();

	public:
		static allocator * instance();

	public:
		static void * malloc( x::uint64 size );
		static void free(void * ptr );

	public:
		static void * valloc( x::uint64 size, vallocflag_t flag );
		static void vfree( void * ptr, x::uint64 size );

	private:
		static heap * get_default_heap();
		static page * get_page( void * ptr );
		static segment * get_segment( void * ptr );
		static x::uint8 get_pages_index( x::uint64 size );

	private:
		void free_collect();
		void * heap_malloc( heap * h, x::uint64 size );
		void * heap_malloc_small( heap * h, x::uint64 size );
		void * heap_malloc_generic( heap * h, x::uint64 size );

	private:
		private_p * _p;
	};
}
