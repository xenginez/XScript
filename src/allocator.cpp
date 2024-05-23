#include "allocator.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


struct x::allocator::private_p
{

};

x::allocator::allocator()
	: _p( new private_p )
{
}

x::allocator::~allocator()
{
	delete _p;
}

void * x::allocator::alloc( x::uint64 size )
{
	return nullptr;
}

void x::allocator::free( void * ptr, x::uint64 size )
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
