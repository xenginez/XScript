#include "library.h"

#ifdef _WIN32
#include <Windows.h>

void * OpenLibrary( const char * name )
{
	return LoadLibraryA( name );
}

void * GetProcess( void * lib, const char * name )
{
	return GetProcAddress( (HMODULE)lib, name );
}

void CloseLibrary( void * lib )
{
	FreeLibrary( (HMODULE)lib );
}

#else

void * OpenLibrary( const char * name )
{

}

void * GetProcess( void * lib, const char * name )
{

}

void CloseLibrary( void * lib )
{

}

#endif // _WIN32

bool x::library::is_open() const
{
	return _lib != nullptr;
}

bool x::library::open( x::static_string_view libname )
{
	char name[256];
	memset( name, 0, 256 );
	memcpy( name, libname.data(), libname.size() );
	_lib = OpenLibrary( name );
	return _lib != nullptr;
}

void x::library::close()
{
	if ( _lib != nullptr )
	{
		CloseLibrary( _lib );
	}
}

void * x::library::get_proc_address( x::static_string_view procname )
{
	auto it = _processes.find( procname );

	if ( it == _processes.end() )
	{
		char name[256];
		memset( name, 0, 256 );
		memcpy( name, procname.data(), procname.size() );
		auto proc = GetProcess( _lib, name );
		if ( proc != nullptr )
			it = _processes.insert( { procname, proc } ).first;
		else
			return nullptr;
	}

	return it->second;
}
