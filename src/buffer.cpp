#include "buffer.h"

#include "value.h"
#include "object.h"
#include "allocator.h"
#include "exception.h"

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

void x::stream_buffer::resize( x::uint64 size )
{
	_data.resize( size, (x::byte)0 );
}

x::uint64 x::stream_buffer::seekp() const
{
	return _ppos;
}

x::uint64 x::stream_buffer::seekg() const
{
	return _gpos;
}

void x::stream_buffer::tellp( x::uint64 i )
{
	_ppos = i;
}

void x::stream_buffer::tellg( x::uint64 i )
{
	_gpos = i;
}

x::uint64 x::stream_buffer::size() const
{
	return _data.size();
}

x::byte * x::stream_buffer::data()
{
	return _data.data();
}

const x::byte * x::stream_buffer::data() const
{
	return _data.data();
}

x::uint64 x::stream_buffer::read( x::value & val )
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

x::uint64 x::stream_buffer::write( const x::value & val )
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

x::uint64 x::stream_buffer::read( x::byte * data, x::uint64 size )
{
	size = std::min( size, _data.size() - _gpos );

	memcpy( data, get( size ), size );

	return size;
}

x::uint64 x::stream_buffer::write( const x::byte * data, x::uint64 size )
{
	auto dst = put( size );
	memcpy( dst, data, size );
	return size;
}

x::byte * x::stream_buffer::prepare( x::uint64 size )
{
	return nullptr;
}

void x::stream_buffer::commit( x::uint64 size )
{
}

void x::stream_buffer::consume()
{
}

x::byte * x::stream_buffer::get( x::uint64 size )
{
	XTHROW( x::runtime_exception, ( _gpos + size ) > _data.size(), "" );

	auto p = _data.data() + _gpos;

	_gpos += size;

	return p;
}

x::byte * x::stream_buffer::put( x::uint64 size )
{
	if ( ( _ppos + size ) > _data.size() )
	{
		_data.resize( _ppos + size );
	}

	auto p = _data.data() + _ppos;

	_ppos += size;

	return p;
}

x::stream_buffer::int_type x::stream_buffer::overflow( int_type _Meta )
{
	// 从放置区写入字符到关联的输出序列
	return int_type();
}

x::stream_buffer::int_type x::stream_buffer::pbackfail( int_type _Meta )
{
	// 将字符放回输入序列，可能修改输入序列
	return int_type();
}

x::stream_buffer::int_type x::stream_buffer::underflow()
{
	// 从关联输入序列读取字符到获取区
	return int_type();
}

x::stream_buffer::pos_type x::stream_buffer::seekoff( off_type _Off, std::ios_base::seekdir _Way, std::ios_base::openmode _Mode )
{
	// 用相对寻址重定位输入序列、输出序列或两者中的下一位置指针
	return pos_type();
}

x::stream_buffer::pos_type x::stream_buffer::seekpos( pos_type _Pos, std::ios_base::openmode _Mode )
{
	// 用绝对寻址重定位输入序列、输出序列或两者中的下一位置指针
	return pos_type();
}
