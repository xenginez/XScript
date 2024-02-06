#include "runtime.h"

#include <deque>

#include "library.h"

using stack_type = std::deque<x::value>;
using thread_type = std::vector<x::value>;
using global_type = std::vector<x::value>;
using library_type = std::map<x::static_string_view, x::library>;

struct x::runtime::private_p
{
	static x::context_ptr _ctx;

	static stack_type & _stack()
	{
		thread_local stack_type stacks;
		return stacks;
	}
	static thread_type & _thread()
	{
		thread_local thread_type threads;
		return threads;
	}
	static global_type & _global()
	{
		static global_type globals;
		return globals;
	}
	static library_type & _library()
	{
		static library_type librarys;
		return librarys;
	}
};

x::runtime::private_p * x::runtime::private_ptr()
{
	static x::runtime::private_p _p;
	return &_p;
}

#define _p private_ptr()
#define _stack _stack()
#define _thread _thread()
#define _global _global()
#define _library _library()

x::runtime::runtime()
{
}

x::runtime::~runtime()
{
}

x::context_ptr x::runtime::context()
{
	return _p->_ctx;
}

void x::runtime::reset( const x::context_ptr & ctx )
{
	_p->_ctx = ctx;

	/// TODO: 
}

void x::runtime::push( value val )
{
	_p->_stack.push_back( val );
}

x::value x::runtime::pop()
{
	auto val = _p->_stack.back();
	_p->_stack.pop_back();
	return val;
}

x::value x::runtime::thread( uint64_t idx )
{
	return _p->_thread[idx];
}

x::value x::runtime::global( uint64_t idx )
{
	return _p->_global[idx];
}

void * x::runtime::alloc( uint64_t size )
{
	return nullptr;
}

void x::runtime::exec_ir( uint64_t code )
{
}

void x::runtime::exec_jit( uint64_t code )
{
}

void x::runtime::exec_aot( uint64_t code )
{
}

void x::runtime::exec_extern( x::static_string_view libname, x::static_string_view procname )
{
	void * proc = nullptr;
	auto it = _p->_library.find( libname );
	if ( it != _p->_library.end() )
	{
		proc = it->second.get_proc_address( procname );
	}
	else
	{
		x::library lib;
		if ( lib.open( libname ) )
		{
			proc = lib.get_proc_address( procname );
			_p->_library.insert( { libname, lib } );
		}
	}

	/// TODO: 

}
