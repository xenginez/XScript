#include "object.h"

#include "meta.h"
#include "value.h"
#include "runtime.h"

void x::object::finalize()
{
}

bool x::object::is_array() const
{
	return false;
}

bool x::object::is_callable() const
{
	return false;
}

bool x::object::is_coroutine() const
{
	return false;
}

x::uint64 x::object::size() const
{
	return x::uint64();
}

x::uint64 x::object::hashcode() const
{
	return x::uint64();
}

x::string x::object::to_string() const
{
	return {};
}

void x::object::copy( x::object * obj )
{
}

void x::object::from_string( x::string str )
{
}

const x::meta_type * x::object::type() const
{
	return nullptr;
}

int x::object::compare( x::object * other ) const
{
	return 0;
}

void x::object::mark( x::runtime * rt )
{
	if ( auto meta = static_cast<const x::meta_class *>( type() ) )
	{
		for ( const auto & it : meta->variables() )
		{
			if ( it->value_type()->type() == x::meta_t::CLASS )
			{
				x::value var;
				if ( it->get( this, var ) )
				{
					rt->add_gray_object( var.to_object() );
				}
			}
		}

		set_gcstatus( x::gcstatus_t::BLACK );
	}
}

x::gcstatus_t x::object::get_gcstatus() const
{
	return (x::gcstatus_t)( *( reinterpret_cast<const x::uint8 *>( this ) - 1 ) & 0x03 );
}

void x::object::set_gcstatus( x::gcstatus_t status )
{
	*( reinterpret_cast<x::uint8 *>( this ) - 1 ) &= static_cast<x::uint8>( status );
}

void x::array_object::finalize()
{

}

bool x::array_object::is_array() const
{
	return true;
}

void x::callable_object::finalize()
{

}

bool x::callable_object::is_callable() const
{
	return true;
}

void x::coroutine_object::finalize()
{

}

bool x::coroutine_object::is_coroutine() const
{
	return true;
}
