#include "runtime.h"

#include <bit>
#include <list>
#include <array>
#include <deque>
#include <bitset>
#include <thread>
#include <thread>
#include <semaphore>
#include <forward_list>
#include <shared_mutex>

namespace
{

	static constexpr x::uint64 item_element_size = 64;
	static constexpr x::uint64 page_element_size = 128;
	static constexpr x::uint64 default_slot_size = 512 + 4 + 4 + 4; // data + bits.data + bits.count + slot.size
	static constexpr x::uint64 default_item_size = default_slot_size * 4 + 4 + 4 + 4;
	static constexpr x::uint64 default_page_size = default_item_size * 4 + 4 + 4 + 4;
	static constexpr x::uint64 default_trigger_size = default_page_size * 1024;
	static constexpr std::array<x::uint64, 32> slot_element_sizes = { 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512 };

	struct bits
	{
		using type = x::uint32;
		static constexpr int BITS_PERWORD = 32;

		x::uint32 _count;
		x::uint32 _data[1];

		static inline constexpr x::uint64 byte_size( x::uint64 bitcount )
		{
			return ( sizeof( _count ) + ( sizeof( bits::type ) * ( ( ( bitcount - 1 ) / bits::BITS_PERWORD ) + 1 ) ) );
		}

		bool all() const
		{
			bool _No_padding = _count % BITS_PERWORD == 0;
			for ( size_t _Wpos = 0; _Wpos < ( ( size_t( _count - 1 ) / BITS_PERWORD ) ) + _No_padding; ++_Wpos )
			{
				if ( _data[_Wpos] != ~type( 0 ) )
				{
					return false;
				}
			}

			return _No_padding || _data[size_t( _count - 1 ) / BITS_PERWORD] == ( type( 1 ) << ( _count % BITS_PERWORD ) ) - 1;
		}
		bool any() const noexcept
		{
			for ( size_t i = 0; i <= ( size_t( _count - 1 ) / BITS_PERWORD ); ++i )
			{
				if ( _data[i] != 0 )
				{
					return true;
				}
			}

			return false;
		}
		bool none() const
		{
			return !any();
		}
		bool get( x::uint64 idx ) const
		{
			return ( _data[idx / BITS_PERWORD] & ( type{ 1 } << idx % BITS_PERWORD ) ) != 0;
		}
		void set( x::uint64 idx, bool val )
		{
			auto & _Selected_word = _data[idx / BITS_PERWORD];
			const auto _Bit = type{ 1 } << idx % BITS_PERWORD;
			if ( val )
				_Selected_word |= _Bit;
			else
				_Selected_word &= ~_Bit;
		}
		x::uint64 next_true( x::uint64 offset ) const;
		x::uint64 next_false( x::uint64 offset ) const;
	};

	struct slot
	{
		x::uint32 _size;
		bits *    _bits;
		x::byte * _data;

		x::byte * alloc();
		void free( void * ptr );
	};

	struct item
	{
		x::uint32 _size;
		bits *    _bits;
		x::byte * _data;

		x::byte * alloc( x::uint64 size );
		void free( void * ptr, x::uint64 size );
	};

	struct page
	{
		x::uint32 _size;
		bits *    _bits;
		x::byte * _data;

		x::byte * alloc( x::uint64 size );
		void free( void * ptr, x::uint64 size );
	};
}

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
	
	std::forward_list<page *> _pages;
	std::forward_list<item *> _items;
	std::array<std::forward_list<slot *>, 32> _slots;

	std::jthread _gcthread;
	std::binary_semaphore _gcsemaphore;

	x::gcstage_t _gcstage = gcstage_t::NONE;
	x::uint64 _gcusesize = 0;
	x::uint64 _gctriggersize = default_trigger_size;
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
			gc_arrange();
		}
	} );
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

x::object * x::runtime::alloc( x::uint64 size )
{
	if ( _p->_gcusesize > _p->_gctriggersize && _p->_gcstage == x::gcstage_t::NONE ) gc();

	size = ALIGN( size, 8 );

	std::shared_lock<std::shared_mutex> lock( _p->_gclock );

	x::byte * ptr = nullptr;

	if ( size <= 512 )
		ptr = slot_alloc( size );
	else
		ptr = item_alloc( size );

	memset( ptr, 0, size );

	return new ( ptr ) x::object();
}

x::byte * x::runtime::page_alloc( x::uint64 size )
{
	for ( auto it : _p->_pages )
	{
		if ( auto ptr = it->alloc( size ) )
			return ptr;
	}

	auto sz = std::max( ALIGN( size, page_element_size ), default_page_size );
	auto ptr = (page *)::malloc( sz );
	ptr->_size = sz;
	ptr->_bits->_count = sz / item_element_size;
	ptr->_data = reinterpret_cast<x::byte *>( ptr ) + sizeof( ptr->_size ) + bits::byte_size( ptr->_bits->_count );
	_p->_pages.push_front( ptr );

	return ptr->alloc( size );
}

x::byte * x::runtime::item_alloc( x::uint64 size )
{
	for ( auto it : _p->_items )
	{
		if ( auto ptr = it->alloc( size ) )
			return ptr;
	}

	auto sz = std::max( ALIGN( size, item_element_size ), default_item_size );
	auto ptr = (item *)page_alloc( sz );
	ptr->_size = sz;
	ptr->_bits->_count = sz / item_element_size;
	ptr->_data = reinterpret_cast<x::byte *>( ptr ) + sizeof( ptr->_size ) + bits::byte_size( ptr->_bits->_count );
	_p->_items.push_front( ptr );

	return ptr->alloc( size );
}

x::byte * x::runtime::slot_alloc( x::uint64 size )
{
	auto & list = _p->_slots[size / 16 - 1];

	for ( auto it : list )
	{
		if ( auto ptr = it->alloc() )
			return ptr;
	}

	auto ptr = (slot *)item_alloc( 1024 );
	ptr->_size = 1024;
	ptr->_bits->_count = 1024 / size;
	ptr->_data = reinterpret_cast<x::byte *>( ptr ) + sizeof( ptr->_size ) + bits::byte_size(ptr->_bits->_count);
	list.push_front( ptr );

	return ptr->alloc();
}

void x::runtime::add_gray( x::object * gray )
{
	if ( _p->_gcstage == x::gcstage_t::MARKING )
		_p->_gcgrays.push_back( gray );
}

void x::runtime::add_wbarriers( x::object * left, x::object * right )
{
	if ( _p->_gcstage == x::gcstage_t::MARKING )
	{
		left->status = x::gcstatus_t::GRAY;
		_p->_gcbarriers.push_back( left );
	}
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

		_p->_gcsemaphore.release();
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
		obj->status = x::gcstatus_t::BLACK;
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
		obj->status = x::gcstatus_t::BLACK;
	}

	_p->_gcstage = x::gcstage_t::CLEARING;
}

void x::runtime::gc_clearing()
{
	for ( x::uint64 i = 0; i < _p->_slots.size(); i++ )
	{
		x::uint64 size = ( i + 1 ) * 16;

		for ( auto it : _p->_slots[i] )
		{
			for ( x::uint64 j = 0; j < it->_bits->_count; j++ )
			{
				if ( it->_bits->get( j ) )
				{
					auto obj = (x::object *)( it->_data + ( size * j ) );

					if ( obj->status == x::gcstatus_t::BLACK )
					{
						obj->status = x::gcstatus_t::WHITE;
					}
					else if ( obj->status == x::gcstatus_t::WHITE )
					{
						obj->finalize();
						it->_bits->set( j, false );
					}
					else
					{
						ASSERT( obj->status == x::gcstatus_t::GRAY, "" );
					}
				}
			}
		}
	}
	for( auto it : _p->_items )
	{
		for ( x::uint64 j = it->_bits->next_true( 0 ); j < it->_bits->_count; j = it->_bits->next_true( j ) )
		{
			auto obj = (x::object *)( it->_data + ( 8 * j ) );

			j += obj->size() / 8;

			if ( obj->status == x::gcstatus_t::BLACK )
			{
				obj->status = x::gcstatus_t::WHITE;
			}
			else if ( obj->status == x::gcstatus_t::WHITE )
			{
				obj->finalize();
				it->_bits->set( j, false );
			}
			else
			{
				ASSERT( obj->status == x::gcstatus_t::GRAY, "" );
			}
		}
	}

	_p->_gcstage = x::gcstage_t::ARRANGE;
}

void x::runtime::gc_arrange()
{
	std::unique_lock<std::shared_mutex> lock( _p->_gclock );

	for ( size_t i = 0; i < _p->_slots.size(); i++ )
	{
		for ( auto list_it = _p->_slots[i].begin(); list_it != _p->_slots[i].end(); )
		{
			if ( ( *list_it )->_bits->none() )
			{
				auto item_it = std::find_if( _p->_items.begin(), _p->_items.end(), [iter = reinterpret_cast<std::intptr_t>( *list_it )]( item * val )
				{ return iter >= reinterpret_cast<std::intptr_t>( val ) && iter < ( reinterpret_cast<std::intptr_t>( iter ) + val->_size ); } );
				if ( item_it != _p->_items.end() )
				{
					( *item_it )->free( ( *list_it ), ( *list_it )->_size );

					list_it = _p->_slots[i].erase_after( list_it );

					continue;
				}
			}

			++list_it;
		}
	}

	for ( auto list_it = _p->_items.begin(); list_it != _p->_items.end(); )
	{
		if ( ( *list_it )->_bits->none() )
		{
			auto page_it = std::find_if( _p->_pages.begin(), _p->_pages.end(), [iter = reinterpret_cast<std::intptr_t>( *list_it )]( page * val )
			{ return iter >= reinterpret_cast<std::intptr_t>( val ) && iter < ( reinterpret_cast<std::intptr_t>( iter ) + val->_size ); } );
			if ( page_it != _p->_pages.end() )
			{
				_p->_gcusesize -= ( *list_it )->_size;

				( *page_it )->free( ( *list_it ), ( *list_it )->_size );

				list_it = _p->_items.erase_after( list_it );
			}
			else
			{
				++list_it;
			}
		}
	}

	for ( auto list_it = _p->_pages.begin(); list_it != _p->_pages.end(); )
	{
		if ( ( *list_it )->_bits->none() )
		{
			::free( ( *list_it ) );

			list_it = _p->_pages.erase_after( list_it );
		}
		else
		{
			++list_it;
		}
	}

	_p->_gcstage = x::gcstage_t::NONE;
}
