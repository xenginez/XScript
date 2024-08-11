#include "buffer.h"

#include "value.h"
#include "object.h"
#include "allocator.h"
#include "exception.h"

namespace
{
	enum
	{
		_Constant = 2,
		_Noread = 4,
		_Append = 8,
		_Atend = 16,
		_From_rvalue = 32
	};
}

void x::buffer::resize( x::uint64 size )
{
	_data.resize( size, (x::byte)0 );
}

x::uint64 x::buffer::seekp() const
{
	return _ppos;
}

x::uint64 x::buffer::seekg() const
{
	return _gpos;
}

void x::buffer::tellp( x::uint64 i )
{
	_ppos = i;
}

void x::buffer::tellg( x::uint64 i )
{
	_gpos = i;
}

x::uint64 x::buffer::size() const
{
	return _data.size();
}

x::byte * x::buffer::data()
{
	return _data.data();
}

const x::byte * x::buffer::data() const
{
	return _data.data();
}

x::uint64 x::buffer::read( x::value & val )
{
	auto gpos = _gpos;

	switch ( val.type() )
	{
	case x::value_t::BOOL:
		val = *reinterpret_cast<bool *>( get( sizeof( bool ) ) );
		break;
	case x::value_t::INT8:
		val = *reinterpret_cast<x::int8 *>( get( sizeof( x::int8 ) ) );
		break;
	case x::value_t::INT16:
		val = *reinterpret_cast<x::int16 *>( get( sizeof( x::int16 ) ) );
		break;
	case x::value_t::INT32:
		val = *reinterpret_cast<x::int32 *>( get( sizeof( x::int32 ) ) );
		break;
	case x::value_t::INT64:
		val = *reinterpret_cast<x::int64 *>( get( sizeof( x::int64 ) ) );
		break;
	case x::value_t::UINT8:
		val = *reinterpret_cast<x::uint8 *>( get( sizeof( x::uint8 ) ) );
		break;
	case x::value_t::UINT16:
		val = *reinterpret_cast<x::uint16 *>( get( sizeof( x::uint16 ) ) );
		break;
	case x::value_t::UINT32:
		val = *reinterpret_cast<x::uint32 *>( get( sizeof( x::uint32 ) ) );
		break;
	case x::value_t::UINT64:
		val = *reinterpret_cast<x::uint64 *>( get( sizeof( x::uint64 ) ) );
		break;
	case x::value_t::FLOAT16:
		val = *reinterpret_cast<x::uint16 *>( get( sizeof( x::uint16 ) ) );
		break;
	case x::value_t::FLOAT32:
		val = *reinterpret_cast<x::float32 *>( get( sizeof( x::float32 ) ) );
		break;
	case x::value_t::FLOAT64:
		val = *reinterpret_cast<x::float64 *>( get( sizeof( x::float64 ) ) );
		break;
	case x::value_t::STRING:
	{
		x::uint32 sz = *reinterpret_cast<x::uint32 *>( get( sizeof( x::uint32 ) ) );

		val = x::allocator::salloc( std::string_view{ reinterpret_cast<const char *>( get( sz ) ), sz } );
	}
		break;
	case x::value_t::OBJECT:
	{
		x::uint32 sz = *reinterpret_cast<x::uint32 *>( get( sizeof( x::uint32 ) ) );

		auto str = x::allocator::salloc( std::string_view{ reinterpret_cast<const char *>( get( sz ) ), sz } );

		val.to_object()->from_string( str );
	}
		break;
	default:
		break;
	}

	return _gpos - gpos;
}

x::uint64 x::buffer::write( const x::value & val )
{
	auto ppos = _ppos;

	switch ( val.type() )
	{
	case x::value_t::BOOL:
		*reinterpret_cast<bool *>( put( sizeof( bool ) ) ) = val.to_bool();
		break;
	case x::value_t::INT8:
		*reinterpret_cast<bool *>( put( sizeof( x::int8 ) ) ) = val.to_int8();
		break;
	case x::value_t::INT16:
		*reinterpret_cast<bool *>( put( sizeof( x::int16 ) ) ) = val.to_int16();
		break;
	case x::value_t::INT32:
		*reinterpret_cast<bool *>( put( sizeof( x::int32 ) ) ) = val.to_int32();
		break;
	case x::value_t::INT64:
		*reinterpret_cast<bool *>( put( sizeof( x::int64 ) ) ) = val.to_int64();
		break;
	case x::value_t::UINT8:
		*reinterpret_cast<bool *>( put( sizeof( x::uint8 ) ) ) = val.to_uint8();
		break;
	case x::value_t::UINT16:
		*reinterpret_cast<bool *>( put( sizeof( x::uint16 ) ) ) = val.to_uint16();
		break;
	case x::value_t::UINT32:
		*reinterpret_cast<bool *>( put( sizeof( x::uint32 ) ) ) = val.to_uint32();
		break;
	case x::value_t::UINT64:
		*reinterpret_cast<bool *>( put( sizeof( x::uint64 ) ) ) = val.to_uint64();
		break;
	case x::value_t::FLOAT16:
		*reinterpret_cast<bool *>( put( sizeof( x::uint16 ) ) ) = val.to_float16().to_uint16();
		break;
	case x::value_t::FLOAT32:
		*reinterpret_cast<bool *>( put( sizeof( x::float32 ) ) ) = val.to_float32();
		break;
	case x::value_t::FLOAT64:
		*reinterpret_cast<bool *>( put( sizeof( x::float64 ) ) ) = val.to_float64();
		break;
	case x::value_t::STRING:
	{
		std::string_view view( val.to_string() );
		*reinterpret_cast<x::uint32 *>( put( sizeof( x::uint32 ) ) ) = static_cast<x::uint32>( view.size() );
		char * buf = reinterpret_cast<char *>( put( view.size() ) );
		memcpy( buf, view.data(), view.size() );
	}
		break;
	case x::value_t::OBJECT:
		write( val.to_object()->to_string() );
		break;
	default:
		break;
	}

	return _ppos - ppos;
}

x::uint64 x::buffer::read( x::byte * data, x::uint64 size )
{
	size = std::min( size, _data.size() - _gpos );

	memcpy( data, get( size ), size );

	return size;
}

x::uint64 x::buffer::write( const x::byte * data, x::uint64 size )
{
	auto dst = put( size );
	memcpy( dst, data, size );
	return size;
}

x::byte * x::buffer::get( x::uint64 size )
{
	XTHROW( x::runtime_exception, ( _gpos + size ) > _data.size(), "" );

	auto p = _data.data() + _gpos;

	_gpos += size;

	return p;
}

x::byte * x::buffer::put( x::uint64 size )
{
	if ( ( _ppos + size ) > _data.size() )
	{
		_data.resize( _ppos + size );
	}

	auto p = _data.data() + _ppos;

	_ppos += size;

	return p;
}


x::stream_buffer::stream_buffer()
	: _Seekhigh( nullptr ), _Mystate( _Getstate( std::ios_base::in | std::ios_base::out ) )
{
}

x::stream_buffer::stream_buffer( stream_buffer && _Right ) noexcept
	: _Mystate( 0 )
{
	_Assign_rv( std::move( _Right ) );
}

x::stream_buffer::stream_buffer( std::ios_base::openmode _Mode )
	: _Seekhigh( nullptr ), _Mystate( _Getstate( _Mode ) )
{
}

x::stream_buffer::stream_buffer( const buf_type & _Str, std::ios_base::openmode _Mode )
{
	_Init( _Str.data(), _Str.size(), _Getstate( _Mode ) );
}

x::stream_buffer & x::stream_buffer::operator=( stream_buffer && _Right ) noexcept
{
	_Assign_rv( std::move( _Right ) );
	return *this;
}

x::stream_buffer::~stream_buffer() noexcept
{
	_Tidy();
}

void x::stream_buffer::swap( stream_buffer & _Right ) noexcept
{
	if ( this != std::addressof( _Right ) )
	{
		streambuf_type::swap( _Right );

		std::swap( _Data, _Right._Data );
		std::swap( _Mystate, _Right._Mystate );
		std::swap( _Seekhigh, _Right._Seekhigh );
	}
}

x::stream_buffer::element_type * x::stream_buffer::prepare( size_type size )
{
	_Prepare.resize( size );
	return _Prepare.data();
}

void x::stream_buffer::commit( size_type size )
{
	this->sputn( _Prepare.data(), size );
	std::move( _Prepare.begin() + size, _Prepare.end(), _Prepare.begin() );
	_Prepare.resize( _Prepare.size() - size );
}

void x::stream_buffer::consume( size_type size )
{
	streambuf_type::setg( streambuf_type::eback(), streambuf_type::gptr() + size, std::max( _Seekhigh, pptr() ) );
}

x::stream_buffer::size_type x::stream_buffer::size() const
{
	return pptr() - gptr();
}

const x::stream_buffer::element_type * x::stream_buffer::data() const
{
	return streambuf_type::gptr();
}

x::stream_buffer::istreambuf_iterator x::stream_buffer::begin()
{
	return istreambuf_iterator( this );
}

x::stream_buffer::istreambuf_iterator x::stream_buffer::end()
{
	return istreambuf_iterator();
}

x::stream_buffer::int_type x::stream_buffer::overflow( int_type _Meta )
{
	if ( _Mystate & _Constant )
	{
		return traits_type::eof();
	}

	if ( traits_type::eq_int_type( traits_type::eof(), _Meta ) )
	{
		return traits_type::not_eof( _Meta );
	}

	const auto _Pptr = streambuf_type::pptr();
	const auto _Epptr = streambuf_type::epptr();
	if ( _Pptr && _Pptr < _Epptr )
	{
		*streambuf_type::_Pninc() = traits_type::to_char_type( _Meta );
		_Seekhigh = _Pptr + 1;
		return _Meta;
	}

	size_t _Oldsize = 0;
	const auto _Oldptr = streambuf_type::eback();
	if ( _Pptr )
	{
		_Oldsize = static_cast<size_t>( _Epptr - _Oldptr );
	}

	size_t _Newsize;
	if ( _Oldsize < _From_rvalue )
	{
		_Newsize = _From_rvalue;
	}
	else if ( _Oldsize < INT_MAX / 2 )
	{
		_Newsize = _Oldsize << 1;
	}
	else if ( _Oldsize < INT_MAX )
	{
		_Newsize = INT_MAX;
	}
	else
	{
		return traits_type::eof();
	}

	_Data.resize( _Newsize );
	const auto _Newptr = _Data.data();

	const auto _New_pnext = _Newptr + _Oldsize;
	_Seekhigh = _New_pnext + 1;

	streambuf_type::setp( _Newptr, _New_pnext, _Newptr + _Newsize );
	if ( _Mystate & _Noread )
	{
		streambuf_type::setg( _Newptr, _Newptr, _Newptr );
	}
	else
	{
		streambuf_type::setg( _Newptr, _Newptr + ( streambuf_type::gptr() - _Oldptr ), _Seekhigh );
	}

	*streambuf_type::_Pninc() = traits_type::to_char_type( _Meta );
	return _Meta;
}

x::stream_buffer::int_type x::stream_buffer::pbackfail( int_type _Meta )
{
	const auto _Gptr = streambuf_type::gptr();
	if ( !_Gptr || _Gptr <= streambuf_type::eback() || ( !traits_type::eq_int_type( traits_type::eof(), _Meta ) && !traits_type::eq( traits_type::to_char_type( _Meta ), _Gptr[-1] ) && ( _Mystate & _Constant ) ) )
	{
		return traits_type::eof();
	}

	streambuf_type::gbump( -1 );

	if ( !traits_type::eq_int_type( traits_type::eof(), _Meta ) )
	{
		*streambuf_type::gptr() = traits_type::to_char_type( _Meta );
	}

	return traits_type::not_eof( _Meta );
}

x::stream_buffer::int_type x::stream_buffer::underflow()
{
	const auto _Gptr = streambuf_type::gptr();
	if ( !_Gptr )
	{
		return traits_type::eof();
	}

	if ( _Gptr < streambuf_type::egptr() )
	{
		return traits_type::to_int_type( *_Gptr );
	}

	const auto _Pptr = streambuf_type::pptr();
	if ( !_Pptr || ( _Mystate & _Noread ) )
	{
		return traits_type::eof();
	}

	const auto _Local_highwater = (std::max)( _Seekhigh, _Pptr );
	if ( _Local_highwater <= _Gptr )
	{
		return traits_type::eof();
	}

	_Seekhigh = _Local_highwater;
	streambuf_type::setg( streambuf_type::eback(), streambuf_type::gptr(), _Local_highwater );
	return traits_type::to_int_type( *streambuf_type::gptr() );
}

x::stream_buffer::pos_type x::stream_buffer::seekoff( off_type _Off, std::ios_base::seekdir _Way, std::ios_base::openmode _Mode )
{
	const bool _Need_read_but_cannot = ( _Mode & std::ios_base::in ) != 0 && ( _Mystate & _Noread ) != 0;
	const bool _Need_write_but_cannot = ( _Mode & std::ios_base::out ) != 0 && ( _Mystate & _Constant ) != 0;
	if ( _Need_read_but_cannot || _Need_write_but_cannot )
	{
		return pos_type{ off_type{ -1 } };
	}

	const auto _Gptr_old = streambuf_type::gptr();
	const auto _Pptr_old = ( _Mystate & _Constant ) ? nullptr : streambuf_type::pptr();
	if ( _Pptr_old && _Seekhigh < _Pptr_old )
	{
		_Seekhigh = _Pptr_old;
	}

	const auto _Seeklow = streambuf_type::eback();
	const auto _Seekdist = _Seekhigh - _Seeklow;
	off_type _Newoff;
	switch ( _Way )
	{
	case std::ios_base::beg:
		_Newoff = 0;
		break;
	case std::ios_base::end:
		_Newoff = _Seekdist;
		break;
	case std::ios_base::cur:
	{
		constexpr auto _Both = std::ios_base::in | std::ios_base::out;
		if ( ( _Mode & _Both ) != _Both )
		{
			if ( _Mode & std::ios_base::in )
			{
				if ( _Gptr_old || !_Seeklow )
				{
					_Newoff = _Gptr_old - _Seeklow;
					break;
				}
			}
			else if ( ( _Mode & std::ios_base::out ) && ( _Pptr_old || !_Seeklow ) )
			{
				_Newoff = _Pptr_old - _Seeklow;
				break;
			}
		}
	}

	_FALLTHROUGH;
	default:
		return pos_type{ off_type{-1} };
	}

	if ( static_cast<unsigned long long>( _Off ) + _Newoff > static_cast<unsigned long long>( _Seekdist ) )
	{
		return pos_type{ off_type{-1} };
	}

	_Off += _Newoff;
	if ( _Off != 0 && ( ( ( _Mode & std::ios_base::in ) && !_Gptr_old ) || ( ( _Mode & std::ios_base::out ) && !_Pptr_old ) ) )
	{
		return pos_type{ off_type{-1} };
	}

	const auto _Newptr = _Seeklow + _Off;
	if ( ( _Mode & std::ios_base::in ) && _Gptr_old )
	{
		streambuf_type::setg( _Seeklow, _Newptr, _Seekhigh );
	}

	if ( ( _Mode & std::ios_base::out ) && _Pptr_old )
	{
		streambuf_type::setp( _Seeklow, _Newptr, streambuf_type::epptr() );
	}

	return pos_type{ _Off };
}

x::stream_buffer::pos_type x::stream_buffer::seekpos( pos_type _Pos, std::ios_base::openmode _Mode )
{
	const bool _Need_read_but_cannot = ( _Mode & std::ios_base::in ) != 0 && ( _Mystate & _Noread ) != 0;
	const bool _Need_write_but_cannot = ( _Mode & std::ios_base::out ) != 0 && ( _Mystate & _Constant ) != 0;
	if ( _Need_read_but_cannot || _Need_write_but_cannot )
	{
		return pos_type{ off_type{-1} };
	}

	const auto _Off = static_cast<std::streamoff>( _Pos );
	const auto _Gptr_old = streambuf_type::gptr();
	const auto _Pptr_old = ( _Mystate & _Constant ) ? nullptr : streambuf_type::pptr();
	if ( _Pptr_old && _Seekhigh < _Pptr_old )
	{
		_Seekhigh = _Pptr_old;
	}

	const auto _Seeklow = streambuf_type::eback();
	const auto _Seekdist = _Seekhigh - _Seeklow;
	if ( static_cast<unsigned long long>( _Off ) > static_cast<unsigned long long>( _Seekdist ) )
	{
		return pos_type{ off_type{-1} };
	}

	if ( _Off != 0 && ( ( ( _Mode & std::ios_base::in ) && !_Gptr_old ) || ( ( _Mode & std::ios_base::out ) && !_Pptr_old ) ) )
	{
		return pos_type{ off_type{-1} };
	}

	const auto _Newptr = _Seeklow + _Off;
	if ( ( _Mode & std::ios_base::in ) && _Gptr_old )
	{
		streambuf_type::setg( _Seeklow, _Newptr, _Seekhigh );
	}

	if ( ( _Mode & std::ios_base::out ) && _Pptr_old )
	{
		streambuf_type::setp( _Seeklow, _Newptr, streambuf_type::epptr() );
	}

	return pos_type{ _Off };
}

void x::stream_buffer::_Init( const element_type * _Ptr, const size_type _Count, int _State )
{
	_State &= ~_From_rvalue;

	if ( _Count != 0 && ( _State & ( _Noread | _Constant ) ) != ( _Noread | _Constant ) )
	{
		size_type _Newsize = _Count;
		_Data.resize( _Newsize );
		const auto _Pnew = _Data.data();

		traits_type::copy( _Pnew, _Ptr, _Count );
		_Seekhigh = _Pnew + _Newsize;

		if ( !( _State & _Noread ) )
		{
			streambuf_type::setg( _Pnew, _Pnew, _Seekhigh );
		}

		if ( !( _State & _Constant ) )
		{
			streambuf_type::setp( _Pnew, ( _State & ( _Atend | _Append ) ) ? _Seekhigh : _Pnew, _Seekhigh );

			if ( _State & _Noread )
			{
				streambuf_type::setg( _Pnew, _Pnew, _Pnew );
			}
		}
	}
	else
	{
		_Seekhigh = nullptr;
	}

	_Mystate = _State;
}

void x::stream_buffer::_Assign_rv( stream_buffer && _Right ) noexcept
{
	if ( this != std::addressof( _Right ) )
	{
		_Tidy();
		this->swap( _Right );
	}
}

void x::stream_buffer::_Tidy() noexcept
{
	streambuf_type::setg( nullptr, nullptr, nullptr );
	streambuf_type::setp( nullptr, nullptr );
	_Data.clear();
	_Data.shrink_to_fit();
	_Seekhigh = nullptr;
}

int x::stream_buffer::_Getstate( std::ios_base::openmode _Mode ) noexcept
{
	int _State = 0;

	if ( !( _Mode & std::ios_base::in ) )
	{
		_State |= _Noread;
	}

	if ( !( _Mode & std::ios_base::out ) )
	{
		_State |= _Constant;
	}

	if ( _Mode & std::ios_base::app )
	{
		_State |= _Append;
	}

	if ( _Mode & std::ios_base::ate )
	{
		_State |= _Atend;
	}

	return _State;
}