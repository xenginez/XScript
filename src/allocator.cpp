#include "allocator.h"

#include <list>
#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#define SMALL_PAGE_INDEX ( 128U )
#define HUGE_PAGE_INDEX ( 73U )
#define FULL_PAGE_INDEX ( 74U )

#define PAGE_DIRECT_SIZE ( SMALL_PAGE_INDEX + 1 )
#define DEFULAT_PAGE_SIZE ( 64 * KB )
#define DEFULAT_SEGMENT_SIZE ( 4 * MB )

#define SMALL_PAGE_SIZE ( 64 * KB )
#define LARGE_PAGE_SIZE ( 512 * KB )
#define HUGE_PAGE_SIZE ( 4 * MB )

#define SMALL_MAX_SIZE ( 16 * KB )
#define MEDIUM_MAX_SIZE ( 128 * KB )
#define LARGE_MAX_SIZE ( 4 * MB )

#define IS_SMALL( SIZE ) ( ( SIZE ) > 0 && ( SIZE ) <= SMALL_MAX_SIZE )
#define IS_MEDIUM( SIZE ) ( ( SIZE ) > SMALL_MAX_SIZE && ( SIZE ) <= MEDIUM_MAX_SIZE )
#define IS_LARGE( SIZE ) ( ( SIZE ) > MEDIUM_MAX_SIZE && ( SIZE ) <= LARGE_MAX_SIZE )
#define IS_HUGE( SIZE ) ( ( SIZE ) > LARGE_MAX_SIZE )

namespace
{
	template<typename T> struct queue
	{
		T push_head( T val )
		{
			T tmp = head;
			while ( !head.compare_exchange_weak( tmp, val ) );
			return tmp;
		}
		T push_tail( T val)
		{
			T tmp = tail;
			while ( !tail.compare_exchange_weak( tmp, val ) );
			return tmp;
		}

		std::atomic<T> head = nullptr;
		std::atomic<T> tail = nullptr;
	};
}

struct x::allocator::block
{
	x::allocator::block *			next = nullptr;
};
struct x::allocator::tld
{
	tld()
	{
		tid = std::this_thread::get_id();
	}

	~tld()
	{
		if ( heap != nullptr ) heap->detach = true;
	}

	std::thread::id					tid = {};
	x::allocator::heap *			heap = nullptr;
};
struct x::allocator::heap
{
	x::allocator::heap *			prev = nullptr;
	x::allocator::heap *			next = nullptr;
	std::thread::id					tid = {};
	bool							detach = false;
	x::allocator::block *			used_block = nullptr;
	x::allocator::page *			direct[PAGE_DIRECT_SIZE] = { nullptr };
	queue<x::allocator::page *>		pages[FULL_PAGE_INDEX + 1] = {};
	queue<x::allocator::segment *>	segments = {};
};
struct x::allocator::page
{
	x::allocator::page *			prev = nullptr;
	x::allocator::page *			next = nullptr;
	x::uint64						size = 0;
	x::uint8						index = 0;
	x::uint64						capacity = 0;
	x::uint64						block_size = 0;
	x::uint16						used = 0;
	x::allocator::block *			free = nullptr;
};
struct x::allocator::segment
{
	x::allocator::segment *			prev = nullptr;
	x::allocator::segment *			next = nullptr;
	std::thread::id					tid = {};
	x::pagekind_t					kind = {};
	x::uint16						psize = 0;
	x::uint16						pused = 0;
	x::uint64						shift = 0;
	x::allocator::page				pages[1] = {};
};

struct x::allocator::private_p
{
	static x::allocator::tld * _tld()
	{
		thread_local x::allocator::tld t_tld;
		return &t_tld;
	}

	std::atomic<x::allocator::heap *> _heaps = nullptr;
};

x::allocator::allocator()
	: _p( new private_p )
{
}

x::allocator::~allocator()
{
	delete _p;
}

void * x::allocator::valloc( x::uint64 size, x::valloc_flags flags )
{
#ifdef _WIN32
	auto protect = PAGE_READWRITE;
	
	if( flags && x::valloc_t::READ ) protect |= PAGE_READONLY;
	if( flags && x::valloc_t::WRITE ) protect |= PAGE_WRITECOPY;
	if( flags && x::valloc_t::EXECUTE ) protect |= PAGE_EXECUTE;

	return VirtualAlloc( nullptr, size, MEM_RESERVE | MEM_COMMIT, protect );
#else
#endif
}

void x::allocator::vfree( void * ptr, x::uint64 size )
{
	if ( ptr == nullptr )
		return;

#ifdef _WIN32
	VirtualFree( ptr, 0, MEM_RELEASE );
#else
#endif
}

void * x::allocator::malloc( x::uint64 size )
{
	return instance()->heap_malloc( instance()->get_default_heap(), size );
}

void x::allocator::free_collect()
{
}

x::allocator::page * x::allocator::get_page( void * ptr )
{
	auto seg = get_segment( ptr );
	return &seg->pages[( ( (std::uintptr_t)ptr ) - ( ( (std::uintptr_t)seg ) ) >> seg->shift )];
}

x::allocator::segment * x::allocator::get_segment( void * ptr )
{
	return (segment *)( (std::uintptr_t)ptr & ~DEFULAT_SEGMENT_SIZE );
}

x::uint8 x::allocator::get_pages_index( x::uint64 size )
{
	x::uint8 idx = 0;

	if ( size <= 1024 )
	{
		idx = (x::uint8)( ( size + 7 ) >> 3 );
	}
	else
	{
		x::uint64 wsize = WSIZE( size );

		if ( wsize <= 1 )
			idx = 1;
		else if ( wsize > ( LARGE_MAX_SIZE / 2 / sizeof( std::uintptr_t ) ) )
			idx = HUGE_PAGE_INDEX;
		else
			idx = (x::uint8)( ( wsize + 1 ) & ~1 );
	}

	return idx;
}

x::allocator * x::allocator::instance()
{
	static x::allocator _allocator;
	return &_allocator;
}

x::allocator::heap * x::allocator::heap_alloc()
{
	heap * h = new heap;

	h->tid = std::this_thread::get_id();

	h->next = _p->_heaps;
	while ( !_p->_heaps.compare_exchange_weak( h->next, h ) );
	if ( h->next != nullptr ) h->next->prev = h;

	return h;
}

x::allocator::heap * x::allocator::find_free_heap()
{
	heap * h = _p->_heaps;

	while ( h != nullptr )
	{
		if ( h->detach )
			return h;
		h = h->next;
	}

	return nullptr;
}

x::allocator::heap * x::allocator::get_default_heap()
{
	auto _tld = private_p::_tld();

	if ( _tld->heap == nullptr )
	{
		_tld->heap = find_free_heap();
		if ( _tld->heap == nullptr )
		{
			_tld->heap = heap_alloc();
		}
	}

	return _tld->heap;
}

void * x::allocator::heap_malloc( x::allocator::heap * h, x::uint64 size )
{
	if ( IS_SMALL( size ) )
		return heap_malloc_small( h, size );
	return heap_malloc_generic( h, size );
}

void * x::allocator::heap_malloc_small( x::allocator::heap * h, x::uint64 size )
{
	page * p = nullptr;
	{

	}

	return page_malloc( h, p, size );
}

void * x::allocator::heap_malloc_generic( x::allocator::heap * h, x::uint64 size )
{
	page * p = nullptr;
	{

	}

	return page_malloc( h, p, size );
}

void * x::allocator::page_malloc( x::allocator::heap * h, x::allocator::page * p, x::uint64 size )
{
	return nullptr;
}

x::allocator::segment * x::allocator::segment_alloc( x::allocator::heap * h, x::uint64 size )
{
	segment * s = new ( valloc( size, { x::valloc_t::READ, x::valloc_t::WRITE } ) ) segment;

	s->tid = std::this_thread::get_id();

	s->next = h->segments.push_head( s );
	if ( s->next ) s->next->prev = s;

	return s;
}

x::allocator::page * x::allocator::segment_alloc_page( x::allocator::heap * h, x::allocator::segment * s, x::uint64 size )
{
	if ( IS_SMALL( size ) )
		return segment_alloc_page_small( h, s );
	else if ( IS_LARGE( size ) )
		return segment_alloc_page_large( h, s );

	return segment_alloc_page_huge( h, s, size );
}

x::allocator::page * x::allocator::segment_alloc_page_small( x::allocator::heap * h, x::allocator::segment * s )
{
	x::allocator::segment * ss = s;

	while ( ss != nullptr )
	{
		if ( ss->kind == x::pagekind_t::SMALL )
		{
			if ( ss->pused - ss->psize > 0 )
			{
				for ( size_t i = 0; i < ss->psize; i++ )
				{
					if ( ss->pages[i].used == 0 )
						return;
				}
			}
		}

		ss = ss->next;
	}

	auto ns = segment_alloc( h, DEFULAT_SEGMENT_SIZE );



	return nullptr;
}

x::allocator::page * x::allocator::segment_alloc_page_large( x::allocator::heap * h, x::allocator::segment * s )
{
	return nullptr;
}

x::allocator::page * x::allocator::segment_alloc_page_huge( x::allocator::heap * h, x::allocator::segment * s, x::uint64 size )
{
	return nullptr;
}
