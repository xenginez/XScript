#pragma once

#include "type.h"

#include <filesystem>
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
		static void * valloc( x::uint64 size, x::valloc_flags flags );
		static void vfree( void * ptr, x::uint64 size );

	public:
		static void * malloc( x::uint64 size );
		static x::string salloc( std::string_view str );

	public:
		static void free_collect();

	private:
		static page * get_page( void * ptr );
		static segment * get_segment( void * ptr );
		static x::uint8 get_pages_index( x::uint64 size );

	private:
		static allocator * instance();

	private:
		heap * heap_alloc();
		heap * find_free_heap();
		heap * get_default_heap();

	private:
		void * heap_malloc( heap * h, x::uint64 size );
		void * heap_malloc_small( heap * h, x::uint64 size );
		void * heap_malloc_generic( heap * h, x::uint64 size );

	private:
		void * page_malloc( heap * h, page * p, x::uint64 size );

	private:
		segment * segment_alloc( heap * h, x::uint64 size );
		page * segment_alloc_page( heap * h, segment * s, x::uint64 size );
		page * segment_alloc_page_small( heap * h, segment * s );
		page * segment_alloc_page_large( heap * h, segment * s );
		page * segment_alloc_page_huge( heap * h, segment * s, x::uint64 size );

	private:
		private_p * _p;
	};
}
