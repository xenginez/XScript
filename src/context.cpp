#include "context.h"

x::context::context()
{
}

x::context::~context()
{

}

int x::context::version() const
{
	return _version;
}

x::meta_ptr x::context::find_meta( x::uint64 hashcode ) const
{
	auto it = _metas.find( hashcode );
	return it == _metas.end() ? nullptr : it->second;
}

x::meta_ptr x::context::find_meta( std::string_view fullname ) const
{
	return find_meta( x::hash( fullname ) );
}

void x::context::load( std::istream & in )
{
}

void x::context::save( std::ostream & out ) const
{
}
