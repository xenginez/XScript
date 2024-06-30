#include "xlib.h"

#ifdef _WIN32
	#include "xlib_windows.cpp"
#elif defined( __APPLE__ )
	#include "TargetConditionals.h"
	#if defined( TARGET_IPHONE_SIMULATOR ) || defined( TARGET_OS_IPHONE )
		#include "xlib_ios.cpp"
	#elif defined( TARGET_OS_MAC )
		#include "xlib_macos.cpp"
	#else
		#error "not support apple platform"
	#endif
#elif defined( __ANDROID__ )
	#include "xlib_android.cpp"
#elif defined( __linux__ )
	#include "xlib_linux.cpp"
#elif defined( __EMSCRIPTEN__ )
	#error "not support emscripten"
#else
	#error "unknown platform"
#endif

#include <atomic>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <shared_mutex>
#include <condition_variable>

#include "buffer.h"
#include "allocator.h"
#include "exception.h"

#define STR_VIEW( S ) x::allocator::transform( std::bit_cast<x::string>( S ) )
#define VIEW_STR( V ) std::bit_cast<x_string>( x::allocator::transform( V ) );

namespace
{
	struct file_info
	{
		uint32 m;
		std::string p;
		std::fstream fs;
	};
	struct condition_info
	{
		std::mutex lock;
		std::condition_variable var;
	};

	template<typename Clock, typename Duration = typename Clock::duration> int64 time_2_stamp( std::chrono::time_point<Clock, Duration> time )
	{
		std::chrono::time_point<Clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>( time );
		auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>( tp.time_since_epoch() );
		return tmp.count();

	}
	template<typename Clock> std::chrono::time_point<Clock> stamp_2_time( int64 stamp )
	{
		return std::chrono::time_point<Clock>() + std::chrono::milliseconds( stamp );
	}

	static const char * mmmm[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static const char * mmm[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static const char * dddd[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	static const char * ddd[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
}

x_string x_path_app_path()
{
	return VIEW_STR( std::filesystem::current_path().string() );
}
x_string x_path_temp_path()
{
	return VIEW_STR( std::filesystem::temp_directory_path().string() );
}
void x_path_copy( x_string frompath, x_string topath, bool recursive, bool overwrite )
{
	std::error_code ec;
	auto opt = std::filesystem::copy_options::none;

	if ( recursive ) opt |= std::filesystem::copy_options::recursive;
	if ( overwrite ) opt |= std::filesystem::copy_options::overwrite_existing;

	std::filesystem::copy( STR_VIEW( frompath ), STR_VIEW( topath ), opt, ec );

	XTHROW( x::runtime_exception, !ec, ec.message() );
}
void x_path_create( x_string path, bool recursive )
{
	std::error_code ec;

	std::filesystem::create_directories( STR_VIEW( path ), ec );

	XTHROW( x::runtime_exception, !ec, ec.message() );
}
void x_path_remove( x_string path )
{
	std::error_code ec;

	std::filesystem::remove_all( STR_VIEW( path ), ec );

	XTHROW( x::runtime_exception, !ec, ec.message() );
}
bool x_path_exists( x_string path )
{
	return std::filesystem::exists( STR_VIEW( path ) );
}
bool x_path_is_file( x_string path )
{
	return !std::filesystem::is_directory( STR_VIEW( path ) );
}
bool x_path_is_directory( x_string path )
{
	return std::filesystem::is_directory( STR_VIEW( path ) );
}
uint64 x_path_entry_count( x_string path )
{
	std::filesystem::directory_iterator it( STR_VIEW( path ) ), end;
	return std::distance( it, end );
}
x_string x_path_at_entry_name( x_string path, uint64 idx )
{
	std::filesystem::directory_iterator it( STR_VIEW( path ) ), end;

	for ( size_t i = 0; it != end; i++, it++ )
	{
		if ( i == idx )
			return VIEW_STR( it->path().filename().string() );
	}

	return { 0, 0 };
}

x_file x_file_create()
{
	return new file_info;
}
bool x_file_open( x_file file, x_string path, uint32 mode )
{
	auto info = reinterpret_cast<file_info *>( file );

	info->m = mode;
	info->p = STR_VIEW( path );
	info->fs.open( info->p, mode );

	return info->fs.is_open();
}
uint64 x_file_size( x_file file )
{
	return std::filesystem::file_size( reinterpret_cast<file_info *>( file )->p );
}
x_string x_file_name( x_file file )
{
	return VIEW_STR( std::filesystem::path( reinterpret_cast<file_info *>( file )->p ).filename().string() );
}
int64 x_file_time( x_file file )
{
	std::error_code ec;
	
	std::filesystem::file_time_type time = std::filesystem::last_write_time( reinterpret_cast<file_info *>( file )->p, ec );

	XTHROW( x::runtime_exception, !ec, ec.message() );

	return time_2_stamp( time );
}
void x_file_rename( x_file file, x_string name )
{
	std::error_code ec;

	std::filesystem::rename( reinterpret_cast<file_info *>( file )->p, STR_VIEW( name ) );

	XTHROW( x::runtime_exception, !ec, ec.message() );
}
bool x_file_is_eof( x_file file )
{
	return reinterpret_cast<file_info *>( file )->fs.eof();
}
uint64 x_file_read_tell( x_file file )
{
	return reinterpret_cast<file_info *>( file )->fs.tellg();
}
uint64 x_file_write_tell( x_file file )
{
	return reinterpret_cast<file_info *>( file )->fs.tellp();
}
void x_file_read_seek( x_file file, uint32 off, uint32 pos )
{
	reinterpret_cast<file_info *>( file )->fs.seekg( off, pos - 1 );
}
void x_file_write_seek( x_file file, uint32 off, uint32 pos )
{
	reinterpret_cast<file_info *>( file )->fs.seekp( off, pos - 1 );
}
uint64 x_file_read( x_file file, intptr buffer, uint64 size )
{
	return reinterpret_cast<file_info *>( file )->fs.readsome( reinterpret_cast<char *>( buffer ), size );
}
uint64 x_file_write( x_file file, intptr buffer, uint64 size )
{
	reinterpret_cast<file_info *>( file )->fs.write( reinterpret_cast<const char *>( buffer ), size );
	return size;
}
x_buffer x_file_read_all( x_file file )
{
	auto sz = x_file_size( file );

	auto buff = new x::buffer;

	buff->resize( sz );

	reinterpret_cast<file_info *>( file )->fs.read( reinterpret_cast<char *>( buff->data() ), sz );

	return buff;
}
void x_file_close( x_file file )
{
	reinterpret_cast<file_info *>( file )->fs.close();
}
void x_file_release( x_file file )
{
	delete reinterpret_cast<file_info *>( file );
}

int64 x_time_now()
{
	return time_2_stamp( std::chrono::system_clock::now() );
}
int32 x_time_year( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(int)ymd.year();
}
int32 x_time_month( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(unsigned int)ymd.month();
}
int32 x_time_day( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(unsigned int)ymd.day();
}
int32 x_time_hour( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return tm.hours().count();
}
int32 x_time_minute( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return tm.minutes().count();
}
int32 x_time_second( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return (int32)tm.seconds().count();
}
int32 x_time_millisecond( int64 time )
{
	auto tp = stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return (int32)tm.subseconds().count();
}
int64 x_time_add_year( int64 time, int32 year )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::years( year ) );
}
int64 x_time_add_month( int64 time, int32 month )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::months( month ) );
}
int64 x_time_add_day( int64 time, int32 day )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::days( day ) );
}
int64 x_time_add_hour( int64 time, int32 hour )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::hours( hour ) );
}
int64 x_time_add_minute( int64 time, int32 minute )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::minutes( minute ) );
}
int64 x_time_add_second( int64 time, int32 second )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::seconds( second ) );
}
int64 x_time_add_millisecond( int64 time, int32 millisecond )
{
	return time_2_stamp( stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::milliseconds( millisecond ) );
}
int64 x_time_second_clock( int64 lefttime, int64 righttime )
{
	return std::chrono::duration_cast<std::chrono::seconds>( stamp_2_time<std::chrono::system_clock>( lefttime ) - stamp_2_time<std::chrono::system_clock>( righttime ) ).count();
}
int64 x_time_millisecond_clock( int64 lefttime, int64 righttime )
{
	return std::chrono::duration_cast<std::chrono::milliseconds>( stamp_2_time<std::chrono::system_clock>( lefttime ) - stamp_2_time<std::chrono::system_clock>( righttime ) ).count();
}
int64 x_time_to_utc( int64 time )
{
	return time_2_stamp( std::chrono::utc_clock::from_sys( stamp_2_time<std::chrono::system_clock>( time ) ) );
}
int64 x_time_from_utc( int64 time )
{
	return time_2_stamp( std::chrono::utc_clock::to_sys( stamp_2_time<std::chrono::utc_clock>( time ) ) );
}
x_string x_time_to_string( int64 time, x_string fmt )
{
	std::string fmt_view( STR_VIEW( fmt ) );

	std::tm tm;
	std::time_t tt = std::chrono::system_clock::to_time_t( stamp_2_time<std::chrono::system_clock>( time ) );

	gmtime_s( &tm, &tt );

	std::ostringstream oss;

	oss << std::put_time( &tm, fmt_view.c_str() );
	
	return VIEW_STR( oss.str() );
}
int64 x_time_from_string( x_string str, x_string fmt )
{
	std::string str_view( STR_VIEW( str ) );
	std::string fmt_view( STR_VIEW( fmt ) );

	std::istringstream iss( str_view );
	
	std::chrono::system_clock::time_point time;
	
	iss >> std::chrono::parse( fmt_view, time );

	return time_2_stamp( time );
}

x_lock x_lock_create()
{
	return new std::shared_mutex;
}
void x_lock_unique_lock( x_lock lock )
{
	reinterpret_cast<std::shared_mutex *>( lock )->lock();
}
void x_lock_unique_unlock( x_lock lock )
{
	reinterpret_cast<std::shared_mutex *>( lock )->unlock();
}
bool x_lock_unique_trylock( x_lock lock )
{
	return reinterpret_cast<std::shared_mutex *>( lock )->try_lock();
}
void x_lock_shared_lock( x_lock lock )
{
	reinterpret_cast<std::shared_mutex *>( lock )->lock_shared();
}
void x_lock_shared_unlock( x_lock lock )
{
	reinterpret_cast<std::shared_mutex *>( lock )->unlock_shared();
}
bool x_lock_shared_trylock( x_lock lock )
{
	return reinterpret_cast<std::shared_mutex *>( lock )->try_lock_shared();
}
void x_lock_release( x_lock lock )
{
	delete reinterpret_cast<std::shared_mutex *>( lock );
}

x_atomic x_atomic_create()
{
	return new std::atomic<intptr>;
}
bool x_atomic_compare_exchange( x_atomic atomic, intptr exp, intptr val )
{
	return reinterpret_cast<std::atomic<intptr> *>( atomic )->compare_exchange_weak( exp, val );
}
intptr x_atomic_exchange( x_atomic atomic, intptr val )
{
	return reinterpret_cast<std::atomic<intptr> *>( atomic )->exchange( val );
}
intptr x_atomic_load( x_atomic atomic )
{
	return reinterpret_cast<std::atomic<intptr> *>( atomic )->load();
}
void x_atomic_store( x_atomic atomic, intptr val )
{
	reinterpret_cast<std::atomic<intptr> *>( atomic )->store( val );
}
void x_atomic_release( x_atomic atomic )
{
	delete reinterpret_cast<std::atomic<intptr> *>( atomic );
}

x_condition x_condition_create()
{
	return new condition_info;
}
void x_condition_wait( x_condition cond )
{
	auto c = reinterpret_cast<condition_info *>( cond );
	std::unique_lock<std::mutex> lock( c->lock );
	c->var.wait( lock );
}
void x_condition_wait_for( x_condition cond, uint64 milliseconds )
{
	auto c = reinterpret_cast<condition_info *>( cond );
	std::unique_lock<std::mutex> lock( c->lock );
	c->var.wait_for( lock, std::chrono::milliseconds( milliseconds ) );
}
void x_condition_notify_one( x_condition cond )
{
	reinterpret_cast<condition_info *>( cond )->var.notify_one();
}
void x_condition_notify_all( x_condition cond )
{
	reinterpret_cast<condition_info *>( cond )->var.notify_all();
}
void x_condition_release( x_condition cond )
{
	delete reinterpret_cast<condition_info *>( cond );
}
