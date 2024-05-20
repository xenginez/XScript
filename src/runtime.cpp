#include "runtime.h"

#include <bit>
#include <array>
#include <deque>
#include <bitset>
#include <thread>
#include <shared_mutex>

namespace
{
	static constexpr x::uint64 trigger_usemem_size = 1024 * 4; // 
	static constexpr std::array<x::uint64, 32> memory_size_list = { 8, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496 };

	struct bits
	{
		x::uint8 _data[1];
	};

	struct slot
	{
		slot *  _next;
		bits    _bits;
		x::byte _data[1];

		x::byte * alloc();
	};

	struct item
	{
		bits    _bits;
		x::byte _data[1];

		x::byte * alloc();
	};

	struct page
	{
		x::uint32 _size;
		bits      _bits;
		x::byte   _data[1];
	};
}

struct x::runtime::private_p
{
	std::vector<page *> _pages;
	std::vector<item *> _items;
	std::array<slot *, 32> _slots;

	x::gcstage_t _gcstage = gcstage_t::NONE;
	x::uint64 _gcusesize = 0;
	std::shared_mutex _gclock;
	std::deque<x::object *> _gcroots;
	std::deque<x::object *> _gcgrays;
	std::deque<x::object *> _gcbarriers;

	std::deque<x::value> _stack;
	std::vector<x::value> _global;
	std::map<std::thread::id, std::vector<x::value>> _thread;
};

x::runtime::runtime()
	: _p( new private_p )
{
}

x::runtime::~runtime()
{
	delete _p;
}

void x::runtime::push( const x::value & val )
{
	_p->_stack.push_back( val );
}

x::value & x::runtime::top()
{
	return _p->_stack.back();
}

x::value x::runtime::pop()
{
	auto val = _p->_stack.back();
	_p->_stack.pop_back();
	return val;
}

x::uint64 x::runtime::pushed()
{
	return _p->_stack.size();
}

void x::runtime::poped( x::uint64 val )
{
	for ( auto i = _p->_stack.size(); i > val; --i )
	{
		auto value = pop();

		/* ... */
	}
}

void x::runtime::resize_global( x::uint64 size )
{
	_p->_global.resize( size );
}

void x::runtime::resize_thread( x::uint64 size )
{
	_p->_thread[std::this_thread::get_id()].resize( size );
}

x::value & x::runtime::get_global( x::uint64 idx )
{
	return _p->_global[idx];
}

x::value & x::runtime::get_thread( x::uint64 idx )
{
	return _p->_thread[std::this_thread::get_id()][idx];
}

x::byte * x::runtime::alloc( x::uint64 size )
{
	if ( _p->_gcusesize > trigger_usemem_size && _p->_gcstage == x::gcstage_t::NONE )
		gc();

	size = ALIGN( size, 8 );

	x::byte * ptr = nullptr;

	std::shared_lock<std::shared_mutex> lock( _p->_gclock );

	if ( size < 512 )
	{
		ptr = _p->_slots.at( ( size / 16 ) + 1 )->alloc();
	}
	else
	{
		for ( auto it : _p->_items )
		{
			if ( ptr = it->alloc() )
				break;
		}
	}

	if ( ptr == nullptr )
	{
		// page alloc
	}

	// memset( ptr, 0, size );

	return ptr;
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

void x::runtime::add_write_barriers( x::object * obj )
{
	_p->_gcgrays.push_back( obj );
}

void x::runtime::gc()
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
		for ( auto & it : _p->_thread )
		{
			for ( auto & it2 : it.second )
			{
				if ( it2.is_object() )
					_p->_gcgrays.push_back( it2.to_object() );
			}
		}
		_p->_gcgrays.insert( _p->_gcgrays.end(), _p->_gcroots.begin(), _p->_gcroots.end() );

		std::thread( [this]()
		{
			gc_marking();
			gc_tracking();
			gc_clearing();
			gc_arrange();
		} ).detach();
	}
}

void x::runtime::gc_marking()
{

	_p->_gcstage = x::gcstage_t::TRACKING;
}

void x::runtime::gc_tracking()
{
	std::unique_lock<std::shared_mutex> lock( _p->_gclock );

	_p->_gcbarriers;

	_p->_gcstage = x::gcstage_t::CLEARING;
}

void x::runtime::gc_clearing()
{


	_p->_gcstage = x::gcstage_t::ARRANGE;
}

void x::runtime::gc_arrange()
{


	_p->_gcstage = x::gcstage_t::NONE;
}
