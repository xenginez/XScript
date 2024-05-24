#include "allocator.h"

#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#define SMALL_PAGE_INDEX ( 128U )
#define HUGE_PAGE_INDEX ( 73U )
#define FULL_PAGE_INDEX ( 74U )

#define SMALL_MAX_SIZE ( 16 * KB )
#define MEDIUM_MAX_SIZE ( 128 * KB )
#define LARGE_MAX_SIZE ( 4 * MB )
#define PAGES_DIRECT_MAX_SIZE ( SMALL_PAGE_INDEX + 1 )

#define IS_SMALL( SIZE ) ( ( SIZE ) > 0 && ( SIZE ) <= SMALL_MAX_SIZE )
#define IS_MEDIUM( SIZE ) ( ( SIZE ) > SMALL_MAX_SIZE && ( SIZE ) < MEDIUM_MAX_SIZE )
#define IS_LARGE( SIZE ) ( ( SIZE ) > MEDIUM_MAX_SIZE && ( SIZE ) <= LARGE_MAX_SIZE )
#define IS_HUGE( SIZE ) ( ( SIZE ) > LARGE_MAX_SIZE )

namespace
{
	template<typename T> struct queue
	{
		T * begin = nullptr;
		T * end = nullptr;
	};
}

struct x::allocator::tld
{
	x::uint64						heartbeat = 0;							// monotonic heartbeat count
	bool							recurse = false;						// true if deferred was called; used to prevent infinite recursion.
	x::allocator::heap *			heap_backing = nullptr;					// backing heap of this thread (cannot be deleted)
	x::allocator::heap *			heaps = nullptr;						// list of heaps in this thread (so we can abandon all when the thread terminates)
	queue<x::allocator::segment>	small_free = {};						// queue of segments with free small pages
	queue<x::allocator::segment>	medium_free = {};						// queue of segments with free medium pages
	queue<x::allocator::page>		pages_purge = {};						// queue of freed pages that are delay purged
	x::uint64						count = 0;								// current number of segments;
	x::uint64						peak_count = 0;							// peak number of segments
	x::uint64						current_size = 0;						// current size of all segments
	x::uint64						peak_size = 0;							// peak size of all segments
	x::uint64						reclaim_count = 0;						// number of reclaimed (abandoned) segments
};
struct x::allocator::heap
{
	tld *							tld = nullptr;
	std::atomic<block *>			thread_delayed_free = nullptr;
	std::thread::id					thread_id = {};							// thread this heap belongs too
//	mi_arena_id_t					arena_id;								// arena id if the heap belongs to a specific arena (or 0)
	std::uintptr_t					cookie = 0;								// random cookie to verify pointers (see `_mi_ptr_cookie`)
	std::uintptr_t					keys[2] = {};							// two random keys used to encode the `thread_delayed_free` list
	x::uint64						page_count = 0;							// total number of pages in the `pages` queues.
	x::uint64						page_retired_min = 0;					// smallest retired index (retired pages are fully free, but still in the page queues)
	x::uint64						page_retired_max = 0;					// largest retired index into the `pages` array.
	x::allocator::heap *			next = nullptr;							// list of heaps per thread
	bool							no_reclaim = false;						// `true` if this heap should not reclaim abandoned pages
	x::uint8						tag = 0;								// custom tag, can be used for separating heaps based on the object types
	x::allocator::page *			pages_free_direct[PAGES_DIRECT_MAX_SIZE];// optimize: array where every entry points a page with possibly free blocks in the corresponding queue for that size.
	queue<x::allocator::page>		pages[FULL_PAGE_INDEX + 1];				// queue of pages for each size class (or "bin")
};
struct x::allocator::page
{
	x::uint8						segment_idx = 0;						// index in the segment `pages` array, `page == &segment->pages[page->segment_idx]`
	x::uint8						segment_in_use : 1 = 0;					// `true` if the segment allocated this page
	x::uint8						is_committed : 1 = 0;					// `true` if the page virtual memory is committed
	x::uint8						is_zero_init : 1 = 0;					// `true` if the page was initially zero initialized
	x::uint8						is_huge : 1 = 0;						// `true` if the page is in a huge segment
	x::uint16						capacity = 0;							// number of blocks committed, must be the first field, see `segment.c:page_clear`
	x::uint16						reserved = 0;							// number of blocks reserved in memory
	x::uint8						flags = 0;								// `in_full` and `has_aligned` flags (8 bits)
	x::uint8						free_is_zero : 1 = 0;					// `true` if the blocks in the free list are zero initialized
	x::uint8						retire_expire : 7 = 0;					// expiration count for retired blocks
	x::allocator::block *			free = nullptr;							// list of available free blocks (`malloc` allocates from this list)
	x::allocator::block *			local_free = nullptr;					// list of deferred free blocks by this thread (migrates to `free`)
	x::uint16						used = 0;								// number of blocks in use (including blocks in `thread_free`)
	x::uint8						block_size_shift = 0;					// if not zero, then `(1 << block_size_shift) == block_size` (only used for fast path in `free.c:_mi_page_ptr_unalign`)
	x::uint8						heap_tag = 0;							// tag of the owning heap, used for separated heaps by object type
	x::uint64						block_size = 0;							// size available in each block (always `>0`)
	x::uint8 *						page_start = nullptr;					// start of the page area containing the blocks
	std::uintptr_t					keys[2] = {};							// two random keys to encode the free lists (see `_mi_block_next`) or padding canary
	std::atomic<std::uintptr_t>		xthread_free = 0;						// list of deferred free blocks freed by other threads
	std::atomic<std::uintptr_t>		xheap = 0;
	x::allocator::page *			next = nullptr;							// next page owned by the heap with the same `block_size`
	x::allocator::page *			prev = nullptr;							// previous page owned by the heap with the same `block_size`
};
struct x::allocator::block
{
	std::uintptr_t					next = 0;
};
struct x::allocator::segment
{
	bool							allow_decommit = false;
	bool							allow_purge = false;
	x::uint64						segment_size = 0;						// for huge pages this may be different from `MI_SEGMENT_SIZE`
	x::allocator::segment *			next = nullptr;							// must be the first segment field after abandoned_next -- see `segment.c:segment_init`
	x::allocator::segment *			prev = nullptr;
	bool							was_reclaimed = false;					// true if it was reclaimed (used to limit on-free reclamation)
	x::uint64						abandoned = 0;							// abandoned pages (i.e. the original owning thread stopped) (`abandoned <= used`)
	x::uint64						abandoned_visits = 0;					// count how often this segment is visited in the abandoned list (to force reclaim if it is too long)
	x::uint64						used = 0;								// count of pages in use (`used <= capacity`)
	x::uint64						capacity = 0;							// count of available pages (`#free + used`)
	x::uint64						segment_info_size = 0;					// space we are using from the first page for segment meta-data and possible guard pages.
	std::uintptr_t					cookie = 0;								// verify addresses in secure mode: `_mi_ptr_cookie(segment) == segment->cookie`
	std::atomic<std::thread::id>	thread_id = {};							// unique id of the thread owning this segment
	x::uint64						page_shift = 0;							// `1 << page_shift` == the page sizes == `page->block_size * page->reserved` (unless the first page, then `-segment_info_size`).
	page_kind_t						page_kind = {};							// kind of pages: small, medium, large, or huge
	x::allocator::page				pages[1] = {};							// up to `MI_SMALL_PAGES_PER_SEGMENT` pages
};
struct x::allocator::private_p
{
	x::allocator::tld * thread_local_data()
	{
		thread_local x::allocator::tld _tld;
		return &_tld;
	}

	x::allocator::segment * _segments = nullptr;
};

x::allocator::allocator()
	: _p( new private_p )
{
}

x::allocator::~allocator()
{
	delete _p;
}

x::allocator * x::allocator::instance()
{
	return nullptr;
}

void * x::allocator::malloc( x::uint64 size )
{
	return instance()->heap_malloc( get_default_heap(), size );
}

void x::allocator::free( void * ptr )
{
}

void * x::allocator::valloc( x::uint64 size, vallocflag_t flag )
{
#ifdef _WIN32
	auto protect = PAGE_READWRITE;

	switch ( flag )
	{
	case x::vallocflag_t::READ: protect = PAGE_READONLY; break;
	case x::vallocflag_t::WRITE: protect = PAGE_WRITECOPY; break;
	case x::vallocflag_t::EXECUTE: protect = PAGE_EXECUTE; break;
	case x::vallocflag_t::READWRITE: protect = PAGE_READWRITE; break;
	case x::vallocflag_t::EXECUTE_READ: protect = PAGE_EXECUTE_READ; break;
	case x::vallocflag_t::EXECUTE_WRITE: protect = PAGE_EXECUTE_WRITECOPY; break;
	case x::vallocflag_t::EXECUTE_READWRITE: protect = PAGE_EXECUTE_READWRITE; break;
	}

	return VirtualAlloc( nullptr, size, MEM_COMMIT, protect );
#else
#endif
}

void x::allocator::vfree( void * ptr, x::uint64 size )
{
#ifdef _WIN32
	VirtualFree( ptr, 0, MEM_RELEASE );
#else
#endif
}

x::allocator::heap * x::allocator::get_default_heap()
{
	return nullptr;
}

x::allocator::page * x::allocator::get_page( void * ptr )
{
	auto seg = get_segment( ptr );
	return &seg->pages[( (std::uintptr_t)(ptr)-(std::uintptr_t)( seg ) ) >> seg->page_shift];
}

x::allocator::segment * x::allocator::get_segment( void * ptr )
{
	return (segment *)( (std::uintptr_t)ptr & ~( 4 * MB ) );
}

x::uint8 x::allocator::get_pages_index( x::uint64 size )
{
	x::uint8 bin = 0;

	if ( size <= 1024 )
	{
		bin = (x::uint8)( ( size + 7 ) >> 3 );
	}
	else
	{
		x::uint64 wsize = ( size + sizeof( std::uintptr_t ) - 1 ) / sizeof( std::uintptr_t );

		if ( wsize <= 1 )
			bin = 1;
		else if ( wsize > ( LARGE_MAX_SIZE / 2 / sizeof( std::uintptr_t ) ) )
			bin = HUGE_PAGE_INDEX;
		else
			bin = (x::uint8)( ( wsize + 1 ) & ~1 );
	}

	return bin;
}

void x::allocator::free_collect()
{
}

void * x::allocator::heap_malloc( heap * h, x::uint64 size )
{
	return nullptr;
}

void * x::allocator::heap_malloc_small( heap * h, x::uint64 size )
{
	return nullptr;
}

void * x::allocator::heap_malloc_generic( heap * h, x::uint64 size )
{
	return nullptr;
}
