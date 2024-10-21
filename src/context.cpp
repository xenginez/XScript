#include "context.h"

#include "meta.h"
#include "module.h"
#include "exception.h"

namespace
{
	struct module_info
	{
		std::string _strpool;
		x::module_ptr _module;
	};
}

struct x::context::private_p
{
	int _version = 0;
	std::string _strpool;
	std::map<x::uint64, meta *> _metas;
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
