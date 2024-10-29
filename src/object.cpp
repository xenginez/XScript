#include "object.h"

#include "meta.h"
#include "value.h"
#include "runtime.h"

void x::object::construct()
{
}

void x::object::finalize()
{
}

x::uint64 x::object::hashcode() const
{
	return x::uint64();
}

void x::object::copyto( x::object * obj )
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

const x::value & x::object::get( x::string name ) const
{
	return x::value::empty_value;
}

void x::object::set( x::string name, const x::value & val )
{

}

void x::object::invoke( const std::vector<x::value> & args, x::value & result )
{

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

		_gcstatus = x::gcstatus_t::BLACK;
	}
}

bool x::coroutine::done() const
{
	return _status == x::corostatus_t::READY;
}

bool x::coroutine::next() const
{
	return _status == x::corostatus_t::SUSPEND;
}

bool x::coroutine::error() const
{
	return _status == x::corostatus_t::EXCEPT;
}

bool x::coroutine::empty() const
{
	return _status == x::corostatus_t::EMPTY;
}

x::corostatus_t x::coroutine::status() const
{
	return _status;
}

const x::value & x::coroutine::wait() const
{
	return value();
}

const x::value & x::coroutine::value() const
{
	return *( (x::value *)( _data.data() + 1 ) );
}

const x::runtime_exception & x::coroutine::exception() const
{
	return *( (x::runtime_exception *)( _data.data() + 1 ) );
}

void x::coroutine::reset()
{
	switch ( (x::uint8)_data[0] )
	{
	case 1:
		( (x::value *)( _data.data() + 1 ) )->~value();
		break;
	case 2:
		( (x::runtime_exception *)( _data.data() + 1 ) )->~runtime_exception();
		break;
	}

	_step = 0;
	_data[0] = {};
	_status = x::corostatus_t::EMPTY;
}

void x::coroutine::resume( const x::value & val )
{
	switch ( (x::uint8)_data[0] )
	{
	case 1:
		( (x::value *)( _data.data() + 1 ) )->~value();
		break;
	case 2:
		( (x::runtime_exception *)( _data.data() + 1 ) )->~runtime_exception();
		break;
	}

	_data[0] = (x::byte)1;
	new ( _data.data() + 1 )x::value( val );

	++_step;
	_status = x::corostatus_t::SUSPEND;
}

void x::coroutine::except( const x::runtime_exception & val )
{
	switch ( (x::uint8)_data[0] )
	{
	case 1:
		( (x::value *)_data.data() + 1 )->~value();
		break;
	case 2:
		( (x::runtime_exception *)_data.data() + 1 )->~runtime_exception();
		break;
	}

	_data[0] = (x::byte)1;
	new ( _data.data() + 1 )x::runtime_exception( val );

	_status = x::corostatus_t::EXCEPT;
}
