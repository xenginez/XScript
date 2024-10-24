#include "machine.h"

#include "runtime.h"
#include "context.h"

struct x::machine::private_p
{

};

x::machine::machine()
	: _p( new private_p )
{
}

x::machine::~machine()
{
	delete _p;
}

int x::machine::exec( const x::runtime_ptr & rt, const x::context_ptr & ctx )
{
	return 0;
}
