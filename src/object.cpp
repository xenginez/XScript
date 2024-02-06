#include "object.h"

#include "runtime.h"

void * x::object::operator new( size_t size )
{
    return runtime::alloc( size );
}

void x::object::operator delete( void * ptr )
{
}
