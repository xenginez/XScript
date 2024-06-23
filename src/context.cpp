#include "context.h"

#include "meta.h"
#include "module.h"
#include "exception.h"

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

	struct module_info
	{
		std::string _strpool;
		x::module_ptr _module;
	};

	struct library_info
	{
		void * handle = nullptr;
		std::string name;
		std::map<std::string, void *> funcs;
	};
}

struct x::context::private_p
{
	int _version = 0;
	std::string _strpool;
	std::map<x::uint64, meta *> _metas;
	std::map<std::string, library_info> _libs;
	std::map<std::string_view, module_info> _modules;
};

x::context::context()
	: _p( new private_p )
{
}

x::context::~context()
{
	delete _p;
}

int x::context::version() const
{
	return _p->_version;
}

bool x::context::load( const x::module_ptr & val )
{
	auto it = _p->_modules.find( val->name );
	if ( it != _p->_modules.end() )
	{
		if ( val->version <= it->second._module->version )
			return true;
	}

	// load module

	return false;
}

const x::meta * x::context::find_meta( x::uint64 hashcode ) const
{
	auto it = _p->_metas.find( hashcode );
	return it == _p->_metas.end() ? nullptr : it->second;
}

const x::meta * x::context::find_meta( std::string_view fullname ) const
{
	return find_meta( x::hash( fullname ) );
}

void * x::context::dlsym( std::string_view dllname, std::string_view funcname )
{
	auto it = _p->_libs.find( { dllname.data(), dllname.size() } );
	if ( it == _p->_libs.end() )
	{
		auto handle = dll_open( dllname );
		if ( handle != nullptr )
		{
			library_info info;
			info.name = dllname;
			info.handle = handle;

			it = _p->_libs.insert( { info.name, info } ).first;
		}
	}

	auto it2 = it->second.funcs.find( { funcname.data(), funcname.size() } );
	if ( it2 == it->second.funcs.end() )
	{
		auto func = dll_sym( it->second.handle, funcname );
		if ( func != nullptr )
		{
			it2 = it->second.funcs.insert( { std::string( funcname.data(), funcname.size() ), func } ).first;
		}
	}

	return it2 != it->second.funcs.end() ? it2->second : nullptr;
}

void x::context::dlclose()
{
	for ( const auto & it : _p->_libs )
	{
		if ( it.second.handle != nullptr )
		{
			XTHROW( x::runtime_exception, dll_close( it.second.handle ), "" );
		}
	}

	_p->_libs.clear();
}
