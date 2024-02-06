#include "xstdlib.h"

#include <tuple>
#include <limits>
#include <format>

#include "value.h"
#include "context.h"

namespace std
{
	template <typename T, typename _CharT> struct formatter<std::span<T>, _CharT> : std::formatter<bool, _CharT>
	{
		template <typename _FormatContext> typename _FormatContext::iterator format( const std::span<T> & v, _FormatContext & format_context ) const
		{
			auto it = std::format_to( format_context.out(), "[" );
			for ( size_t i = 0; i < v.size(); i++ )
			{
				it = std::formatter<T, _CharT>().format( v[i], format_context );
				if ( i < v.size() - 1 )
					it = std::format_to( it, ", " );
			}
			return std::format_to( it, "]" );
		}
	};
	template <typename _CharT> struct formatter<x::value, _CharT> : std::formatter<bool, _CharT>
	{
		template <typename _FormatContext> typename _FormatContext::iterator format( const x::value & v, _FormatContext & format_context ) const
		{
			switch ( v.type() )
			{
			case x::value_t::NIL:
				return std::formatter<std::nullptr_t, _CharT>().format( nullptr, format_context );
			case x::value_t::BYTE:
				return std::formatter<int8_t, _CharT>().format( v.get<int8_t>(), format_context );
			case x::value_t::BOOLEAN:
				return std::formatter<bool, _CharT>().format( v.get<bool>(), format_context );
			case x::value_t::INTEGER:
				return std::formatter<int64_t, _CharT>().format( v.get<int64_t>(), format_context );
			case x::value_t::FLOATING:
				return std::formatter<double, _CharT>().format( v.get<double>(), format_context );
			case x::value_t::ANY:
				break;
			case x::value_t::ARRAY:
				return std::formatter<std::span<x::value>, _CharT>().format( v.get<std::span<x::value>>(), format_context );
			case x::value_t::STRING:
				return std::formatter<std::string_view, _CharT>().format( v.get<std::string_view>(), format_context );
			case x::value_t::NATIVE:
				break;
			case x::value_t::SCRIPT:
				break;
			case x::value_t::CLOSURE:
				break;
			case x::value_t::OBJECT:
				break;
			default:
				break;
			}

			return std::format_to( format_context.out(), "invalid" );
		}
	};

	template<size_t N> struct span_to_tuple
	{
		static auto of( std::span<x::value> args )
		{
			return std::tuple_cat( span_to_tuple<N - 1>::of( args ), std::make_tuple( args[N] ) );
		}
	};
	template<> struct span_to_tuple<0>
	{
		static auto of( std::span<x::value> args )
		{
			return std::make_tuple( args[0] );
		}
	};


#define XFORMAT( N ) \
	inline std::string xformat_##N##( std::string_view fmt, std::span<x::value> args ) \
	{ \
		return std::vformat( fmt, std::apply( []( auto && ... arg ) \
		{ \
			return std::make_format_args( std::move( arg )... ); \
		}, span_to_tuple<##N##-1>::of( args ) ) ); \
	}

	XFORMAT( 1 );
	XFORMAT( 2 );
	XFORMAT( 3 );
	XFORMAT( 4 );
	XFORMAT( 5 );
	XFORMAT( 6 );
	XFORMAT( 7 );
	XFORMAT( 8 );
	XFORMAT( 9 );
	XFORMAT( 10 );

#undef XFORMAT
}

x::path::path( const path & val )
	:_path( val._path )
{
}

x::path::path( std::string_view val )
	:_path( val )
{
}

x::path::path( const std::filesystem::path & val )
	:_path( val )
{
}

x::path & x::path::append( std::string_view src )
{
	_path.append( src );
	return *this;
}

void x::path::clear()
{
	_path.clear();
}

x::path & x::path::concat( std::string_view src )
{
	_path.concat( src );
	return *this;
}

int x::path::compare( const path & src ) const
{
	return _path.compare( src._path );
}

std::string x::path::string() const
{
	return _path.string();
}

x::path x::path::filename() const
{
	return _path.filename();
}

x::path x::path::stem() const
{
	return _path.stem();
}

x::path x::path::extension() const
{
	return _path.extension();
}

bool x::path::empty() const
{
	return _path.empty();
}

bool x::path::exists() const
{
	return std::filesystem::exists( _path );
}

bool x::path::is_file() const
{
	return !std::filesystem::is_directory( _path );
}

bool x::path::is_directory() const
{
	return std::filesystem::is_directory( _path );
}

int64_t x::path::file_size() const
{
	return std::filesystem::file_size( _path );
}

bool x::path::remove() const
{
	return std::filesystem::remove( _path );
}

bool x::path::remove_all() const
{
	return std::filesystem::remove_all( _path );
}

void x::path::rename( const path & npath ) const
{
	std::filesystem::rename( _path, npath._path );
}

void x::path::resize( int64_t size ) const
{
	std::filesystem::resize_file( _path, size );
}

bool x::path::create() const
{
	if ( !exists() )
	{
		if ( is_file() )
		{
			std::ofstream ofs( _path );
			ofs.close();
		}
		else
		{
			return std::filesystem::create_directories( _path );
		}
	}

	return true;
}

void x::path::copy( const path & val, copy_options opt ) const
{
	std::filesystem::copy( _path, val._path, (std::filesystem::copy_options)opt );
}

std::vector<x::path> x::path::entrys( directory_options opt ) const
{
	std::vector<path> result;
	for ( std::filesystem::directory_iterator it( _path ), end; it != end; ++it )
	{
		if ( ( it->path().filename() == "." || it->path().filename() == ".." ) && ( opt | directory_options::dot_and_dotdot ) )
			result.push_back( it->path() );
		else if ( std::filesystem::is_directory( it->path() ) && opt | directory_options::directorys )
			result.push_back( it->path() );
		else if ( opt | directory_options::files )
			result.push_back( it->path() );
	}
	return result;
}

x::path x::path::current_path()
{
	return std::filesystem::current_path();
}

x::path x::path::temp_directory_path()
{
	return std::filesystem::temp_directory_path();
}

x::file::file( const path & val, int mode )
	:_file( val.string(), mode )
{
}

bool x::file::is_open() const
{
	return _file.is_open();
}

void x::file::open( const path & val, int mode )
{
	_file.open( val.string(), mode );
}

void x::file::close()
{
	_file.close();
}

void x::file::flush()
{
	_file.flush();
}

int64_t x::file::get()
{
	return _file.get();
}

int64_t x::file::geti8()
{
	return _file.get();
}

int64_t x::file::geti16()
{
	int16_t v;
	_file.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::file::geti32()
{
	int32_t v;
	_file.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::file::geti64()
{
	int64_t v;
	_file.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::file::read( std::span<uint8_t> data, int64_t off, int64_t size )
{
	if ( size == -1 ) size = data.size() - off;
	return (int64_t)_file.readsome( (char *)( data.data() + off ), size );
}

int64_t x::file::tellg()
{
	return _file.tellg();
}

void x::file::seekg( int64_t off, seek_dir dir )
{
	_file.seekg( off, (int)dir );
}

void x::file::put( int64_t c )
{
	_file.put( (char)c );
}

void x::file::puti8( int64_t c )
{
	_file.put( (char)c );
}

void x::file::puti16( int64_t c )
{
	int16_t v = (int16_t)c;
	_file.write( (const char *)&v, sizeof( v ) );
}

void x::file::puti32( int64_t c )
{
	int32_t v = (int32_t)c;
	_file.write( (const char *)&v, sizeof( v ) );
}

void x::file::puti64( int64_t c )
{
	int64_t v = c;
	_file.write( (const char *)&v, sizeof( v ) );
}

void x::file::write( std::span<uint8_t> data, int64_t size )
{
	size = std::min<size_t>( data.size(), size );
	_file.write( (const char *)data.data(), size );
}

int64_t x::file::tellp()
{
	return _file.tellp();
}

void x::file::seekp( int64_t off, seek_dir dir )
{
	_file.seekp( off, (int)dir );
}

x::time x::time::now()
{
	x::time t;
	t._time = clock_type::now();
	return t;
}

int64_t x::time::year() const
{
	return std::chrono::duration_cast<std::chrono::years>( _time.time_since_epoch() ).count();
}

int64_t x::time::month() const
{
	return std::chrono::duration_cast<std::chrono::months>( _time.time_since_epoch() ).count();
}

int64_t x::time::day() const
{
	return std::chrono::duration_cast<std::chrono::days>( _time.time_since_epoch() ).count();
}

int64_t x::time::hour() const
{
	return std::chrono::duration_cast<std::chrono::hours>( _time.time_since_epoch() ).count();
}

int64_t x::time::minute() const
{
	return std::chrono::duration_cast<std::chrono::minutes>( _time.time_since_epoch() ).count();
}

int64_t x::time::second() const
{
	return std::chrono::duration_cast<std::chrono::seconds>( _time.time_since_epoch() ).count();
}

int64_t x::time::millisecond() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>( _time.time_since_epoch() ).count();
}

void x::time::set_year( int64_t val )
{
	_time += std::chrono::years( val - year() );
}

void x::time::set_month( int64_t val )
{
	_time += std::chrono::months( val - month() );
}

void x::time::set_day( int64_t val )
{
	_time += std::chrono::days( val - day() );
}

void x::time::set_hour( int64_t val )
{
	_time += std::chrono::hours( val - hour() );
}

void x::time::set_minute( int64_t val )
{
	_time += std::chrono::minutes( val - minute() );
}

void x::time::set_second( int64_t val )
{
	_time += std::chrono::seconds( val - second() );
}

void x::time::set_millisecond( int64_t val )
{
	_time += std::chrono::milliseconds( val - millisecond() );
}

void x::time::add_year( int64_t val )
{
	_time += std::chrono::years( val );
}

void x::time::add_month( int64_t val )
{
	_time += std::chrono::months( val );
}

void x::time::add_day( int64_t val )
{
	_time += std::chrono::days( val );
}

void x::time::add_hour( int64_t val )
{
	_time += std::chrono::hours( val );
}

void x::time::add_minute( int64_t val )
{
	_time += std::chrono::minutes( val );
}

void x::time::add_second( int64_t val )
{
	_time += std::chrono::seconds( val );
}

void x::time::add_millisecond( int64_t val )
{
	_time += std::chrono::milliseconds( val );
}

int64_t x::time::relative_year( time val ) const
{
	return std::chrono::duration_cast<std::chrono::years>( _time - val._time ).count();
}

int64_t x::time::relative_month( time val ) const
{
	return std::chrono::duration_cast<std::chrono::months>( _time - val._time ).count();
}

int64_t x::time::relative_day( time val ) const
{
	return std::chrono::duration_cast<std::chrono::days>( _time - val._time ).count();
}

int64_t x::time::relative_hour( time val ) const
{
	return std::chrono::duration_cast<std::chrono::hours>( _time - val._time ).count();
}

int64_t x::time::relative_minute( time val ) const
{
	return std::chrono::duration_cast<std::chrono::minutes>( _time - val._time ).count();
}

int64_t x::time::relative_second( time val ) const
{
	return std::chrono::duration_cast<std::chrono::seconds>( _time - val._time ).count();
}

int64_t x::time::relative_millisecond( time val ) const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>( _time - val._time ).count();
}

x::time::clock_type::time_point x::time::time_point() const
{
	return _time;
}

int64_t x::buff::geti8()
{
	return _stream.get();
}

int64_t x::buff::geti16()
{
	int16_t v;
	_stream.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::buff::geti32()
{
	int32_t v;
	_stream.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::buff::geti64()
{
	int64_t v;
	_stream.read( (char *)&v, sizeof( v ) );
	return v;
}

int64_t x::buff::read( std::span<uint8_t> data, int64_t off, int64_t size )
{
	return _stream.readsome( (char *)data.data() + off, size );
}

int64_t x::buff::tellg()
{
	return _stream.tellg();
}

void x::buff::seekg( int64_t off, seek_dir dir )
{
	_stream.seekg( off, dir );
}

void x::buff::puti8( int64_t c )
{
	_stream.put( (char)c );
}

void x::buff::puti16( int64_t c )
{
	int16_t v = (int16_t)c;
	_stream.write( (const char *)&v, sizeof( v ) );
}

void x::buff::puti32( int64_t c )
{
	int32_t v = ( int32_t )c;
	_stream.write( (const char *)&v, sizeof( v ) );
}

void x::buff::puti64( int64_t c )
{
	_stream.write( (const char *)&c, sizeof( c ) );
}

void x::buff::write( std::span<uint8_t> data, int64_t size )
{
	_stream.write( (const char *)data.data(), size );
}

int64_t x::buff::tellp()
{
	return _stream.tellp();
}

void x::buff::seekp( int64_t off, seek_dir dir )
{
	_stream.seekp( off, dir );
}

void x::mutex::lock()
{
	_lock.lock();
}

bool x::mutex::try_lock()
{
	return _lock.try_lock();
}

void x::mutex::unlock()
{
	_lock.unlock();
}

void x::rwmutex::rlock()
{
	_lock.lock_shared();
}

bool x::rwmutex::try_rlock()
{
	return _lock.try_lock_shared();
}

void x::rwmutex::runlock()
{
	_lock.unlock_shared();
}

void x::rwmutex::wlock()
{
	_lock.lock();
}

bool x::rwmutex::try_wlock()
{
	return _lock.try_lock();
}

void x::rwmutex::wunlock()
{
	_lock.unlock();
}

int64_t x::random::device()
{
	static std::random_device dev;
	return dev();
}

void x::random::seed( int64_t val )
{
	_engine.seed( (uint32_t)val );
}

int64_t x::random::irand( int64_t min, int64_t max )
{
	std::uniform_int_distribution<int64_t> dis( min, max );
	return dis( _engine );
}

double x::random::frand( double min, double max )
{
	std::uniform_real_distribution<double> dis( min, max );
	return dis( _engine );
}

double x::random::normal_frand( double min, double max )
{
	std::normal_distribution<double> dis( min, max );
	return dis( _engine );
}

int64_t x::thread::get_id()
{
	return std::bit_cast<int32_t>( std::this_thread::get_id() );
}

void x::thread::sleep_for( int64_t msec )
{
	std::this_thread::sleep_for( std::chrono::milliseconds( msec ) );
}

void x::thread::sleep_until( time val )
{
	std::this_thread::sleep_until( val.time_point() );
}

bool x::thread::stop_requested()
{
	return stop_token().stop_requested();
}

int64_t x::thread::hardware_concurrency()
{
	return std::jthread::hardware_concurrency();
}

std::stop_token & x::thread::stop_token()
{
	thread_local std::stop_token token;
	return token;
}

void x::thread::run()
{
	_thread = std::jthread( []( std::stop_token token )
	{
		stop_token() = std::move( token );

		/// TODO: 

		stop_token() = {};
	} );
}

void x::thread::join()
{
	_thread.join();
}

void x::thread::detach()
{
	_thread.detach();
}

bool x::thread::request_stop()
{
	return _thread.request_stop();
}

inline int64_t x::thread::id() const
{
	return std::bit_cast<int32_t>( _thread.get_id() );
}

bool x::thread::joinable() const
{
	return _thread.joinable();
}

void x::printf( std::string_view fmt, std::span<x::value> args )
{
	std::printf( "%s", x::format( fmt, args ).c_str() );
}

std::string x::format( std::string_view fmt, std::span<x::value> args )
{
	switch ( args.size() )
	{
	case  0: return { fmt.begin(), fmt.end() };
	case  1: return std::xformat_1( fmt, args );
	case  2: return std::xformat_2( fmt, args );
	case  3: return std::xformat_3( fmt, args );
	case  4: return std::xformat_4( fmt, args );
	case  5: return std::xformat_5( fmt, args );
	case  6: return std::xformat_6( fmt, args );
	case  7: return std::xformat_7( fmt, args );
	case  8: return std::xformat_8( fmt, args );
	case  9: return std::xformat_9( fmt, args );
	case 10: return std::xformat_10( fmt, args );
	}

	std::string result( fmt.begin(), fmt.end() );
	for ( size_t i = 0; i < args.size(); i += 10 )
	{
		result = x::format( result, { args.begin() + i, args.begin() + std::min( i + 10, args.size() ) } );
	}
	return result;
}

void x::load_stdlib( const x::context_ptr & ctx )
{
	using int_limits = std::numeric_limits<uint64_t>;
	using float_limits = std::numeric_limits<double>;

	ctx->
	beg_namespace( "std" )
		.beg_enum<seek_dir>( "seek_dir" )
			.element( "beg", seek_dir::beg )
			.element( "cur", seek_dir::cur )
			.element( "end", seek_dir::end )
		.end_enum()
		.beg_enum<open_mode>( "open_mode" )
			.element( "in", open_mode::in )
			.element( "out", open_mode::out )
			.element( "ate", open_mode::ate )
			.element( "app", open_mode::app )
			.element( "trunc", open_mode::trunc )
			.element( "binary", open_mode::binary )
		.end_enum()
		.beg_enum<copy_options>("copy_options")
			.element( "skip_existing", x::copy_options::skip_existing )
			.element( "overwrite_existing", x::copy_options::overwrite_existing )
			.element( "update_existing", x::copy_options::update_existing )
			.element( "directories_only", x::copy_options::directories_only )
			.element( "recursive", x::copy_options::recursive )
		.end_enum()
		.beg_enum<directory_options>("directory_options")
			.element( "recursive", directory_options::files )
			.element( "recursive", directory_options::directorys )
			.element( "recursive", directory_options::dot_and_dotdot )
			.element( "recursive", directory_options::all_entrys )
		.end_enum()
		.beg_namespace( "math" )
			.function( "e", math::e )
			.function( "log2e", math::log2e )
			.function( "log10e", math::log10e )
			.function( "pi", math::pi )
			.function( "inv_pi", math::inv_pi )
			.function( "inv_sqrtpi", math::inv_sqrtpi )
			.function( "ln2", math::ln2 )
			.function( "ln10", math::ln10 )
			.function( "sqrt2", math::sqrt2 )
			.function( "sqrt3", math::sqrt3 )
			.function( "inv_sqrt3", math::inv_sqrt3 )
			.function( "egamma", math::egamma )
			.function( "phi", math::phi )
		.end_namespace()
		.beg_class<int_limits>( "int_limits" )
			.function( "min", &int_limits::min )
			.function( "max", &int_limits::max )
			.function( "epsilon", &int_limits::epsilon )
			.function( "infinity", &int_limits::infinity )
			.function( "quiet_NaN", &int_limits::quiet_NaN )
		.end_class()
		.beg_class<float_limits>( "float_limits" )
			.function( "min", &float_limits::min )
			.function( "max", &float_limits::max )
			.function( "epsilon", &float_limits::epsilon )
			.function( "infinity", &float_limits::infinity )
			.function( "quiet_NaN", &float_limits::quiet_NaN )
		.end_class()
		.beg_class<path>( "path" )
			.function( "append",  &path::append )
			.function( "concat", &path::concat )
			.function( "clear", &path::clear )
			.function( "string", &path::string )
			.function( "compare", &path::compare )
			.function( "filename", &path::filename )
			.function( "stem", &path::stem )
			.function( "extension", &path::extension )
			.function( "empty", &path::empty )
			.function( "exists", &path::exists )
			.function( "is_file", &path::is_file )
			.function( "is_directory", &path::is_directory )
			.function( "file_size", &path::file_size )
			.function( "remove", &path::remove )
			.function( "remove_all", &path::remove_all )
			.function( "rename", &path::rename )
			.function( "resize", &path::resize )
			.function( "create", &path::create )
			.function( "copy", &path::copy )
			.function( "entrys", &path::entrys )
			.function( "current_path", &path::current_path )
			.function( "temp_directory_path", &path::temp_directory_path )
		.end_class()
		.beg_class<file>("file")
			.function( "is_open", &file::is_open )
			.function( "open", &file::open )
			.function( "close", &file::close )
			.function( "flush", &file::flush )
			.function( "get", &file::get )
			.function( "geti8", &file::geti8 )
			.function( "geti16", &file::geti16 )
			.function( "geti32", &file::geti32 )
			.function( "geti64", &file::geti64 )
			.function( "read", &file::read )
			.function( "tellg", &file::tellg )
			.function( "seekg", &file::seekg )
			.function( "put", &file::put )
			.function( "puti8", &file::puti8 )
			.function( "puti16", &file::puti16 )
			.function( "puti32", &file::puti32 )
			.function( "puti64", &file::puti64 )
			.function( "write", &file::write )
			.function( "tellp", &file::tellp )
			.function( "seekp", &file::seekp )
		.end_class()
		.beg_class<time>( "time" )
			.function( "now", &time::now )
			.function( "year", &time::year )
			.function( "month", &time::month )
			.function( "day", &time::day )
			.function( "hour", &time::hour )
			.function( "minute", &time::minute )
			.function( "second", &time::second )
			.function( "millisecond", &time::millisecond )
			.function( "set_year", &time::set_year )
			.function( "set_month", &time::set_month )
			.function( "set_day", &time::set_day )
			.function( "set_hour", &time::set_hour )
			.function( "set_minute", &time::set_minute )
			.function( "set_second", &time::set_second )
			.function( "set_millisecond", &time::set_millisecond )
			.function( "add_year", &time::add_year )
			.function( "add_month", &time::add_month )
			.function( "add_day", &time::add_day )
			.function( "add_hour", &time::add_hour )
			.function( "add_minute", &time::add_minute )
			.function( "add_second", &time::add_second )
			.function( "add_millisecond", &time::add_millisecond )
			.function( "relative_year", &time::relative_year )
			.function( "relative_month", &time::relative_month )
			.function( "relative_day", &time::relative_day )
			.function( "relative_hour", &time::relative_hour )
			.function( "relative_minute", &time::relative_minute )
			.function( "relative_second", &time::relative_second )
			.function( "relative_millisecond", &time::relative_millisecond )
		.end_class()
		.beg_class<buff>( "buff" )
		.end_class()
		.beg_class<mutex>( "mutex" )
			.function( "lock", &mutex::lock )
			.function( "try_lock", &mutex::try_lock )
			.function( "unlock", &mutex::unlock )
		.end_class()
		.beg_class<rwmutex>( "rwmutex" )
			.function( "rlock", &rwmutex::rlock )
			.function( "try_rlock", &rwmutex::try_rlock )
			.function( "runlock", &rwmutex::runlock )
			.function( "wlock", &rwmutex::wlock )
			.function( "try_wlock", &rwmutex::try_wlock )
			.function( "wunlock", &rwmutex::wunlock )
		.end_class()
		.beg_class<random>( "random" )
			.function( "device", &random::device )
			.function( "seed", &random::seed )
			.function( "irand", &random::irand )
			.function( "frand", &random::frand )
			.function( "normal_frand", &random::normal_frand )
		.end_class()
		.beg_class<thread>( "thread" )
			.function( "get_id", &thread::get_id )
			.function( "sleep_for", &thread::sleep_for )
			.function( "sleep_until", &thread::sleep_until )
			.function( "stop_requested", &thread::stop_requested )
			.function( "hardware_concurrency", &thread::hardware_concurrency )
			.function( "run", &thread::run )
			.function( "join", &thread::join )
			.function( "detach", &thread::detach )
			.function( "request_stop", &thread::request_stop )
			.function( "id", &thread::id )
			.function( "joinable", &thread::joinable )
		.end_class()
		.function("printf", x::printf )
		.function("format", x::format )
	.end_namespace();
}
