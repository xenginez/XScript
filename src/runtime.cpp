#include "runtime.h"

#include <bit>
#include <list>
#include <array>
#include <deque>
#include <bitset>
#include <thread>
#include <semaphore>
#include <shared_mutex>

#include "allocator.h"

struct x::runtime::tld
{
	tld( x::runtime * rt )
		: _rt( rt )
	{
		_rt->insert_thread_runtime( this );
	}
	~tld()
	{
		_rt->remove_thread_runtime( this );
	}

	static tld * current()
	{
		return *current_ptr();
	}
	static tld ** current_ptr()
	{
		thread_local tld * data = nullptr;
		return &data;
	}

	x::runtime * _rt = nullptr;
	std::vector<x::value> _thread;
	std::deque<x::value> _valstack;
	std::array<x::byte, 1 * MB> _memstack = {};
};

struct x::runtime::private_p
{
	private_p()
		: _gcsemaphore( 0 )
	{
		_gcsemaphore.acquire();
	}
	~private_p()
	{
		_gcthread.request_stop();
		_gcsemaphore.release();
	}
	
	std::jthread _gcthread;
	std::binary_semaphore _gcsemaphore;

	x::gcstage_t _gcstage = gcstage_t::NONE;
	x::uint64 _gcusesize = 0;
	x::uint64 _gctriggersize = 0;
	std::shared_mutex _gclock;
	std::deque<x::object *> _gcroots;
	std::deque<x::object *> _gcgrays;
	std::deque<x::object *> _gcbarriers;

	std::vector<x::value> _global;
	std::map<std::thread::id, tld *> _threads;
};

x::runtime::runtime()
	: _p( new private_p )
{
	_p->_gcthread = std::jthread( [this]( std::stop_token token )
	{
		while ( !token.stop_requested() )
		{
			_p->_gcsemaphore.acquire();

			if ( token.stop_requested() )
				return;

			gc_marking();
			gc_tracking();
			gc_clearing();
			gc_collect();
		}
	} );
}

x::runtime::~runtime()
{
	delete _p;
}

x::runtime * x::runtime::thread_owner_runtime()
{
	if ( tld::current() != nullptr )
		return tld::current()->_rt;
	return nullptr;
}

void x::runtime::push( const x::value & val )
{
	tld::current()->_valstack.push_back( val );
}

x::value & x::runtime::top()
{
	return tld::current()->_valstack.back();
}

x::value x::runtime::pop()
{
	auto val = tld::current()->_valstack.back();
	tld::current()->_valstack.pop_back();
	return val;
}

x::uint64 x::runtime::pushed()
{
	return tld::current()->_valstack.size();
}

void x::runtime::poped( x::uint64 val )
{
	tld::current()->_valstack.resize( val );
}

void x::runtime::resize_global( x::uint64 size )
{
	_p->_global.resize( size );
}

void x::runtime::resize_thread( x::uint64 size )
{
	tld::current()->_thread.resize( size );
}

x::value x::runtime::get_global( x::uint64 idx )
{
	return _p->_global[idx];
}

x::value x::runtime::get_thread( x::uint64 idx )
{
	return tld::current()->_thread[idx];
}

void x::runtime::add_root( x::object * root )
{
	for ( size_t i = 0; i < _p->_gcroots.size(); i++ )
	{
		if ( _p->_gcroots[i] == nullptr )
		{
			_p->_gcroots[i] = root;
			return;
		}
	}

	_p->_gcroots.push_back( root );
}

void x::runtime::set_trigger_gc_size( x::uint64 size )
{
	_p->_gctriggersize = size;
}

x::runtime::tld * x::runtime::create_tld()
{
	if ( tld::current() == nullptr )
	{
		*tld::current_ptr() = new tld( this );
	}

	return tld::current();
}

bool x::runtime::delete_tld( tld * val )
{
	if ( tld::current() == val )
	{
		delete val;
		return true;
	}

	return false;
}

x::object * x::runtime::alloc( x::uint64 size )
{
	if ( _p->_gcusesize > _p->_gctriggersize && _p->_gcstage == x::gcstage_t::NONE ) gc();

	return new ( allocator::malloc( ALIGN( size, 8 ) ) ) x::object();
}

void x::runtime::add_gray_object( x::object * gray )
{
	if ( _p->_gcstage == x::gcstage_t::MARKING )
		_p->_gcgrays.push_back( gray );
}

void x::runtime::add_write_barrier( x::object * owner, x::object * val )
{
	if ( _p->_gcstage == x::gcstage_t::MARKING )
	{
		owner->set_gcstatus( x::gcstatus_t::GRAY );
		_p->_gcbarriers.push_back( owner );
	}
}

void x::runtime::gc()
{
	if ( _p->_gcstage == x::gcstage_t::NONE )
	{
		std::unique_lock<std::shared_mutex> lock( _p->_gclock );

		if ( _p->_gcstage == x::gcstage_t::NONE )
		{
			_p->_gcstage = x::gcstage_t::MARKING;

			_p->_gcgrays.clear();
			_p->_gcbarriers.clear();

			for ( auto & it : _p->_global )
			{
				if ( it.is_object() )
					_p->_gcgrays.push_back( it.to_object() );
			}
			for ( auto & it : _p->_threads )
			{
				for ( auto & it2 : it.second->_valstack )
				{
					if ( it2.is_object() )
						_p->_gcgrays.push_back( it2.to_object() );
				}
				for ( auto & it2 : it.second->_thread )
				{
					if ( it2.is_object() )
						_p->_gcgrays.push_back( it2.to_object() );
				}
			}
			_p->_gcgrays.insert( _p->_gcgrays.end(), _p->_gcroots.begin(), _p->_gcroots.end() );

			_p->_gcsemaphore.release();
		}
	}
}

void x::runtime::gc_marking()
{
	x::object * obj = nullptr;

	while ( _p->_gcgrays.empty() )
	{
		obj = _p->_gcgrays.back();
		_p->_gcgrays.pop_back();
		obj->mark( this );
		obj->set_gcstatus( x::gcstatus_t::BLACK );
	}

	_p->_gcstage = x::gcstage_t::TRACKING;
}

void x::runtime::gc_tracking()
{
	std::unique_lock<std::shared_mutex> lock( _p->_gclock );

	_p->_gcbarriers;
	x::object * obj = nullptr;
	while ( _p->_gcbarriers.empty() )
	{
		obj = _p->_gcbarriers.back();
		_p->_gcbarriers.pop_back();
		obj->mark( this );
		obj->set_gcstatus( x::gcstatus_t::BLACK );
	}

	_p->_gcstage = x::gcstage_t::CLEARING;
}

void x::runtime::gc_clearing()
{
	

	_p->_gcstage = x::gcstage_t::COLLECT;
}

void x::runtime::gc_collect()
{
	std::unique_lock<std::shared_mutex> lock( _p->_gclock );

	allocator::instance()->free_collect();

	_p->_gctriggersize = _p->_gcusesize + x::uint64( _p->_gcusesize * 0.2f );

	_p->_gcstage = x::gcstage_t::NONE;
}

void x::runtime::insert_thread_runtime( tld * rt )
{
	_p->_threads[std::this_thread::get_id()] = rt;
}

void x::runtime::remove_thread_runtime( tld * rt )
{
	auto it = _p->_threads.find( std::this_thread::get_id() );
	if ( it != _p->_threads.end() )
		_p->_threads.erase( it );
}
