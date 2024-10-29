#include "runtime.h"

#include <bit>
#include <list>
#include <deque>
#include <bitset>
#include <thread>
#include <semaphore>
#include <shared_mutex>

#include "allocator.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
#ifdef _WIN32
	void * dll_open( std::string_view name )
	{
		if ( name.empty() )
			return GetModuleHandleA( nullptr );

		char buf[512];

		memset( buf, 0, 512 );
		memcpy( buf, name.data(), name.size() );
		buf[name.size()] = 0;

		return LoadLibraryA( buf );
	}
	void * dll_sym( void * handle, std::string_view name )
	{
		char buf[512];

		memset( buf, 0, 512 );
		memcpy( buf, name.data(), name.size() );
		buf[name.size()] = 0;

		return GetProcAddress( reinterpret_cast<HMODULE>( handle ), buf );
	}
	bool dll_close( void * handle )
	{
		return FreeLibrary( reinterpret_cast<HMODULE>( handle ) );
	}
#else
	void * dll_open( std::string_view name )
	{
		if ( name.empty() )
			return nullptr;

		char buf[512];

		memset( buf, 0, 512 );
		memcpy( buf, name.data(), name.size() );
		buf[name.size()] = 0;

		return dlopen( buf, RTLD_LAZY );
	}
	void * dll_sym( void * handle, std::string_view name )
	{
		char buf[512];

		memset( buf, 0, 512 );
		memcpy( buf, name.data(), name.size() );
		buf[name.size()] = 0;

		return dlsym( handle, buf );
	}
	bool dll_close( void * handle )
	{
		return dlclose( handle ) == 0;
	}
#endif

	struct library_info
	{
		void * handle = nullptr;
		std::string name;
		std::map<std::string, void *> funcs;
	};
}

struct x::runtime::tld
{
	struct value
	{
		bool b;
		x::int8 i8;
		x::int16 i16;
		x::int32 i32;
		x::int64 i64;
		x::uint8 u8;
		x::uint16 u16;
		x::uint32 u32;
		x::uint64 u64;
		x::float16 f16;
		x::float32 f32;
		x::float64 f64;
		x::object * obj;
	};
	struct frame
	{
		x::uint64 pc = 0;
	};

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

	std::deque<value> _valstack;
	std::deque<frame> _framestack;
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
	std::map<std::string, library_info> _dlllibs;
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

x::runtime * x::runtime::this_thread_owner_runtime()
{
	if ( tld::current() != nullptr )
		return tld::current()->_rt;
	return nullptr;
}

void x::runtime::push( const x::value & val )
{
	/// TODO: tld::current()->_valstack.push_back( val );
}

x::value x::runtime::pop()
{
	auto val = tld::current()->_valstack.back();
	tld::current()->_valstack.pop_back();
	/// TODO: return val;
	return {};
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

void x::runtime::attach_root( x::object * root )
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

void x::runtime::detach_root( x::object * root )
{
	for ( size_t i = 0; i < _p->_gcroots.size(); i++ )
	{
		if ( _p->_gcroots[i] == root )
		{
			_p->_gcroots[i] = nullptr;
			break;
		}
	}
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

x::object * x::runtime::obj_alloc( x::uint64 size )
{
	if ( _p->_gcusesize > _p->_gctriggersize && _p->_gcstage == x::gcstage_t::NONE ) gc();

	return new ( allocator::malloc( ALIGN( size, 8 ) ) ) x::object();
}

x::coroutine * x::runtime::coro_alloc( x::uint64 size )
{
	if ( _p->_gcusesize > _p->_gctriggersize && _p->_gcstage == x::gcstage_t::NONE ) gc();

	return new ( allocator::malloc( ALIGN( size, 8 ) ) ) x::coroutine();
}

void x::runtime::call_extern_c( std::string_view dllname, std::string_view funcname, x::callmode_t mode, x::uint32 args )
{
	auto dll_it = _p->_dlllibs.find( { dllname.data(), dllname.size() } );
	if ( dll_it == _p->_dlllibs.end() )
	{
		auto handle = dll_open( dllname );
		if ( handle != nullptr )
		{
			library_info info;
			info.name = dllname;
			info.handle = handle;

			dll_it = _p->_dlllibs.insert( { info.name, info } ).first;
		}
	}

	if ( dll_it != _p->_dlllibs.end() )
	{
		auto func_it = dll_it->second.funcs.find( { funcname.data(), funcname.size() } );
		if ( func_it == dll_it->second.funcs.end() )
		{
			auto func = dll_sym( dll_it->second.handle, funcname );
			if ( func != nullptr )
			{
				func_it = dll_it->second.funcs.insert( { std::string( funcname.data(), funcname.size() ), func } ).first;
			}
		}

		if ( func_it != dll_it->second.funcs.end() )
		{
			/// TODO: 
			switch ( mode )
			{
			case x::callmode_t::MODE_C:
				break;
			case x::callmode_t::MODE_STD:
				break;
			case x::callmode_t::MODE_FAST:
				break;
			case x::callmode_t::MODE_THIS:
				break;
			default:
				break;
			}

			return;
		}
	}

	XTHROW( x::runtime_exception, false, "" );
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
		owner->_gcstatus = x::gcstatus_t::GRAY;
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
					/// TODO: 
					// if ( it2.is_object() )
					// 	_p->_gcgrays.push_back( it2.to_object() );
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
		obj->_gcstatus = x::gcstatus_t::BLACK;
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
		obj->_gcstatus = x::gcstatus_t::BLACK;
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
