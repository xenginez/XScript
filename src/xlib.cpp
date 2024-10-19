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
#else
	#error "unknown platform"
#endif

#include <atomic>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <shared_mutex>
#include <condition_variable>

#include <iconv.h>
#include <uchardet.h>

#include "zip.h"
#include "buffer.h"
#include "endian.hpp"
#include "allocator.h"
#include "exception.h"
#include "scheduler.h"

namespace
{
	struct image_info
	{

	};
	struct condition_info
	{
		std::mutex lock;
		std::condition_variable var;
	};

	static int ARGC = 0;
	static const char ** ARGV = nullptr;
	static const char * MMMM[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	static const char * MMM[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static const char * dddd[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday","Sunday" };
	static const char * ddd[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
}

int x_main( int argc, const char ** argv )
{
	ARGC = argc;
	ARGV = argv;

	x_os_init();

	x::scheduler::init();

	while ( x::scheduler::run() );

	return 0;
}
uint64 x_argc()
{
	return ARGC;
}
x_string x_argv( uint64 index )
{
	return ARGV[index];
}

x_zip x_zip_create()
{
	return new x::zip;
}
void x_zip_load( x_zip zip, x_path path )
{
	auto z = (x::zip *)zip;

	z->load( std::filesystem::path( path ) );
}
void x_zip_save( x_zip zip, x_path path )
{
	auto z = (x::zip *)zip;

	z->save( std::filesystem::path( path ) );
}
bool x_zip_exist( x_zip zip, x_string name )
{
	auto z = (x::zip *)zip;
		
	return z->exist( name );
}
uint64 x_zip_file_size( x_zip zip, x_string name )
{
	auto z = (x::zip *)zip;

	return z->file_size( name );
}
uint64 x_zip_compress_size( x_zip zip, x_string name )
{
	auto z = (x::zip *)zip;

	return z->compress_size( name );
}
void x_zip_extract( x_zip zip, x_string name, x_path path )
{
	auto z = (x::zip *)zip;

	z->extract( std::filesystem::path( path ), name );
}
void x_zip_extract_all( x_zip zip, x_path path )
{
	auto z = (x::zip *)zip;

	z->extractall( std::filesystem::path( path ) );
}
uint64 x_zip_read( x_zip zip, x_string name, x_buffer buf )
{
	auto z = (x::zip *)zip;

	std::ostream os( (x::buffer *)buf );

	return z->read( name, os );
}
void x_zip_write( x_zip zip, x_string name, x_buffer buf )
{
	auto z = (x::zip *)zip;

	std::istream is( (x::buffer *)buf );

	z->write( is, name );
}
void x_zip_write_str( x_zip zip, x_string name, x_string str )
{
	auto z = (x::zip *)zip;

	z->write_str( name, str );
}
void x_zip_release( x_zip zip )
{
	delete (x::zip *)zip;
}
void x_zip_compression( x_buffer inbuf, x_buffer outbuf )
{
	std::istream is( (x::buffer *)inbuf );
	std::ostream os( (x::buffer *)outbuf );

	x::zip::compression( is, os );
}
void x_zip_decompression( x_buffer inbuf, x_buffer outbuf )
{
	std::istream is( (x::buffer *)inbuf );
	std::ostream os( (x::buffer *)outbuf );

	x::zip::decompression( is, os );
}

x_path x_path_temp_path()
{
	return x_locale_local_utf8( std::filesystem::temp_directory_path().string().c_str() );
}
x_path x_path_current_path()
{
	return x_locale_local_utf8( std::filesystem::current_path().string().c_str() );
}
void x_path_copy( x_path frompath, x_path topath, bool recursive, bool overwrite )
{
	std::error_code ec;
	auto opt = std::filesystem::copy_options::none;

	if ( recursive ) opt |= std::filesystem::copy_options::recursive;
	if ( overwrite ) opt |= std::filesystem::copy_options::overwrite_existing;

	std::filesystem::copy( frompath, topath, opt, ec );

	XTHROW( x::runtime_exception, ec, ec.message() );
}
void x_path_create( x_path path, bool recursive )
{
	std::error_code ec;

	std::filesystem::create_directories( path, ec );

	XTHROW( x::runtime_exception, ec, ec.message() );
}
void x_path_remove( x_path path )
{
	std::error_code ec;

	std::filesystem::remove_all( path, ec );

	XTHROW( x::runtime_exception, ec, ec.message() );
}
bool x_path_exists( x_path path )
{
	return std::filesystem::exists( path );
}
bool x_path_is_file( x_path path )
{
	return !std::filesystem::is_directory( path );
}
bool x_path_is_directory( x_path path )
{
	return std::filesystem::is_directory( path );
}
uint64 x_path_entry_count( x_path path )
{
	std::filesystem::directory_iterator it( path ), end;
	return std::distance( it, end );
}
x_path x_path_at_entry( x_path path, uint64 idx )
{
	std::filesystem::directory_iterator it( path ), end;

	for ( size_t i = 0; it != end; i++, it++ )
	{
		if ( i == idx )
			return x_locale_local_utf8( it->path().filename().string().c_str() );
	}

	return nullptr;
}

int64 x_time_now()
{
	return x::time_2_stamp( std::chrono::system_clock::now() );
}
int32 x_time_year( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(int)ymd.year();
}
int32 x_time_month( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(unsigned int)ymd.month();
}
int32 x_time_day( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	return (int32)(unsigned int)ymd.day();
}
int32 x_time_weekday( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_weekday ymd{ dp };
	return (int32)ymd.weekday().iso_encoding();
}
int32 x_time_hour( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return tm.hours().count();
}
int32 x_time_minute( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return tm.minutes().count();
}
int32 x_time_second( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return (int32)tm.seconds().count();
}
int32 x_time_millisecond( int64 time )
{
	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	return (int32)tm.subseconds().count();
}
int64 x_time_add_year( int64 time, int32 year )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::years( year ) );
}
int64 x_time_add_month( int64 time, int32 month )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::months( month ) );
}
int64 x_time_add_day( int64 time, int32 day )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::days( day ) );
}
int64 x_time_add_hour( int64 time, int32 hour )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::hours( hour ) );
}
int64 x_time_add_minute( int64 time, int32 minute )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::minutes( minute ) );
}
int64 x_time_add_second( int64 time, int32 second )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::seconds( second ) );
}
int64 x_time_add_millisecond( int64 time, int32 millisecond )
{
	return x::time_2_stamp( x::stamp_2_time<std::chrono::system_clock>( time ) + std::chrono::milliseconds( millisecond ) );
}
int64 x_time_second_clock( int64 lefttime, int64 righttime )
{
	return std::chrono::duration_cast<std::chrono::seconds>( x::stamp_2_time<std::chrono::system_clock>( lefttime ) - x::stamp_2_time<std::chrono::system_clock>( righttime ) ).count();
}
int64 x_time_millisecond_clock( int64 lefttime, int64 righttime )
{
	return std::chrono::duration_cast<std::chrono::milliseconds>( x::stamp_2_time<std::chrono::system_clock>( lefttime ) - x::stamp_2_time<std::chrono::system_clock>( righttime ) ).count();
}
int64 x_time_to_utc( int64 time )
{
	return x::time_2_stamp( std::chrono::utc_clock::from_sys( x::stamp_2_time<std::chrono::system_clock>( time ) ) );
}
int64 x_time_from_utc( int64 time )
{
	return x::time_2_stamp( std::chrono::utc_clock::to_sys( x::stamp_2_time<std::chrono::utc_clock>( time ) ) );
}
x_string x_time_to_string( int64 time, x_string fmt )
{
	std::string buf;
	std::string_view fmt_view( fmt );

	auto tp = x::stamp_2_time<std::chrono::system_clock>( time );
	auto dp = std::chrono::floor<std::chrono::days>( tp );
	std::chrono::year_month_day ymd{ dp };
	std::chrono::year_month_weekday ymwd{ dp };
	std::chrono::hh_mm_ss tm{ std::chrono::floor<std::chrono::milliseconds>( tp - dp ) };
	
	auto year = int( ymd.year() );
	auto month = unsigned int( ymd.month() );
	auto day = unsigned int( ymd.day() );
	auto week = ymwd.weekday().iso_encoding();
	auto hour = tm.hours().count();
	auto minute = tm.minutes().count();
	auto second = tm.seconds().count();
	auto millisecond = tm.subseconds().count();

	/*
		d		日期为数字，不带零（1到31）
		dd		日数以零开头（01到31）
		ddd		星期名称（例如“Mon”到“Sun”）
		dddd	长星期名称（例如“Monday”到“Sunday”）
		M		月份为数字，不带零（1-12）
		MM		月份以零开头（01-12）
		MMM		是本地化月份名称（例如“Jan”到“Dec”）
		MMMM	长本地化月份名称（例如“January”到“December”）
		yy		以两位数表示的年份（00-99）
		yyyy	以四位数表示的年份

		h		不带零的小时（0到23）
		hh		以零开头的小时（00至23）
		m		不带零的分钟（0到59）
		mm		以零开头的分钟（00到59）
		s		不带零的秒（0到59）
		ss		以零开头的秒（00到59）
		z		不带零的毫秒（0到999）
		zzz		以零开头的毫秒（000到999）
	*/

	for ( int i = 0; i < fmt_view.size(); )
	{
		int j = 0;
		char key[5];
		memset( key, 0, 5 );

		if ( ( i < fmt_view.size() ) && fmt_view[i] == 'y' && ( i + 1 ) < fmt_view.size() && fmt_view[i + 1] == 'y' )
		{
			key[j++] = fmt_view[i++];
			key[j++] = fmt_view[i++];

			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'y' && ( i + 1 ) < fmt_view.size() && fmt_view[i + 1] == 'y' )
			{
				key[j++] = fmt_view[i++];
				key[j++] = fmt_view[i++];
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 'M' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'M' )
			{
				key[j++] = fmt_view[i++];
				if ( ( i < fmt_view.size() ) && fmt_view[i] == 'M' )
				{
					key[j++] = fmt_view[i++];
					if ( ( i < fmt_view.size() ) && fmt_view[i] == 'M' )
					{
						key[j++] = fmt_view[i++];
					}
				}
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 'd' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'd' )
			{
				key[j++] = fmt_view[i++];
				if ( ( i < fmt_view.size() ) && fmt_view[i] == 'd' )
				{
					key[j++] = fmt_view[i++];
					if ( ( i < fmt_view.size() ) && fmt_view[i] == 'd' )
					{
						key[j++] = fmt_view[i++];
					}
				}
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 'h' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'h' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 'm' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'm' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 's' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 's' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( ( i < fmt_view.size() ) && fmt_view[i] == 'z' )
		{
			key[j++] = fmt_view[i++];
			if ( ( i < fmt_view.size() ) && fmt_view[i] == 'z' && ( i + 1 < fmt_view.size() && fmt_view[i + 1] == 'z' ) )
			{
				key[j++] = fmt_view[i++];
				key[j++] = fmt_view[i++];
			}
		}
		else
		{
			buf.push_back( fmt_view[i++] );
		}

		if ( j != 0 )
		{
			if ( strcmp( key, "yyyy" ) == 0 )
				buf.append( std::format( "{:04d}", year ) );
			else if ( strcmp( key, "yy" ) == 0 )
				buf.append( std::format( "{}", year ) );
			else if ( strcmp( key, "MMMM" ) == 0 )
				buf.append( MMMM[month] );
			else if ( strcmp( key, "MMM" ) == 0 )
				buf.append( MMM[month] );
			else if ( strcmp( key, "MM" ) == 0 )
				buf.append( std::format( "{:02d}", month ) );
			else if ( strcmp( key, "M" ) == 0 )
				buf.append( std::format( "{}", month ) );
			else if ( strcmp( key, "dddd" ) == 0 )
				buf.append( dddd[week - 1] );
			else if ( strcmp( key, "ddd" ) == 0 )
				buf.append( ddd[week - 1] );
			else if ( strcmp( key, "dd" ) == 0 )
				buf.append( std::format( "{:02d}", day ) );
			else if ( strcmp( key, "d" ) == 0 )
				buf.append( std::format( "{}", day ) );
			else if ( strcmp( key, "hh" ) == 0 )
				buf.append( std::format( "{:02d}", hour ) );
			else if ( strcmp( key, "h" ) == 0 )
				buf.append( std::format( "{}", hour ) );
			else if ( strcmp( key, "mm" ) == 0 )
				buf.append( std::format( "{:02d}", minute ) );
			else if ( strcmp( key, "m" ) == 0 )
				buf.append( std::format( "{}", minute ) );
			else if ( strcmp( key, "ss" ) == 0 )
				buf.append( std::format( "{:02d}", second ) );
			else if ( strcmp( key, "s" ) == 0 )
				buf.append( std::format( "{}", second ) );
			else if ( strcmp( key, "zzz" ) == 0 )
				buf.append( std::format( "{:03d}", millisecond ) );
			else if ( strcmp( key, "z" ) == 0 )
				buf.append( std::format( "{}", millisecond ) );
			else
				buf.append( key );
		}
	}

	return x::allocator::salloc( buf );
}
int64 x_time_from_string( x_string str, x_string fmt )
{
	std::string_view str_view( str );
	std::string_view fmt_view( fmt );

	int64 year = 0;
	int64 month = 0;
	int64 day = 0;
	int64 week = 0;
	int64 hour = 0;
	int64 minute = 0;
	int64 second = 0;
	int64 millisecond = 0;

	/*
		d		日期为数字，不带零（1到31）
		dd		日数以零开头（01到31）
		ddd		星期名称（例如“Mon”到“Sun”）
		dddd	长星期名称（例如“Monday”到“Sunday”）
		M		月份为数字，不带零（1-12）
		MM		月份以零开头（01-12）
		MMM		是本地化月份名称（例如“Jan”到“Dec”）
		MMMM	长本地化月份名称（例如“January”到“December”）
		yy		以两位数表示的年份（00-99）
		yyyy	以四位数表示的年份

		h		不带零的小时（0到23）
		hh		以零开头的小时（00至23）
		m		不带零的分钟（0到59）
		mm		以零开头的分钟（00到59）
		s		不带零的秒（0到59）
		ss		以零开头的秒（00到59）
		z		不带零的毫秒（0到999）
		zzz		以零开头的毫秒（000到999）
	*/

	for ( size_t i = 0, k = 0; i < fmt_view.size(); )
	{
		size_t j = 0;
		char key[5];
		memset( key, 0, 5 );

		if ( fmt_view[i] == 'y' && ( i + 1 ) < fmt_view.size() && fmt_view[i + 1] == 'y' )
		{
			key[j++] = fmt_view[i++];
			key[j++] = fmt_view[i++];

			if ( fmt_view[i] == 'y' && ( i + 1 ) < fmt_view.size() && fmt_view[i + 1] == 'y' )
			{
				key[j++] = fmt_view[i++];
				key[j++] = fmt_view[i++];
			}
		}
		else if ( fmt_view[i] == 'M' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'M' )
			{
				key[j++] = fmt_view[i++];
				if ( fmt_view[i] == 'M' )
				{
					key[j++] = fmt_view[i++];
					if ( fmt_view[i] == 'M' )
					{
						key[j++] = fmt_view[i++];
					}
				}
			}
		}
		else if ( fmt_view[i] == 'd' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'd' )
			{
				key[j++] = fmt_view[i++];
				if ( fmt_view[i] == 'd' )
				{
					key[j++] = fmt_view[i++];
					if ( fmt_view[i] == 'd' )
					{
						key[j++] = fmt_view[i++];
					}
				}
			}
		}
		else if ( fmt_view[i] == 'h' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'h' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( fmt_view[i] == 'H' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'H' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( fmt_view[i] == 'm' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'm' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( fmt_view[i] == 's' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 's' )
			{
				key[j++] = fmt_view[i++];
			}
		}
		else if ( fmt_view[i] == 'z' )
		{
			key[j++] = fmt_view[i++];
			if ( fmt_view[i] == 'z' && ( i + 1 < fmt_view.size() && fmt_view[i + 1] == 'z' ) && ( i + 2 < fmt_view.size() && fmt_view[i + 2] == 'z' ) )
			{
				key[j++] = fmt_view[i++];
				key[j++] = fmt_view[i++];
			}
		}
		else
		{
			i++; k++;
		}

		if ( j != 0 )
		{
			if ( strcmp( key, "yyyy" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 4, year );
				k += 4;
			}
			else if ( strcmp( key, "yy" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, year );
				k += 2;
			}
			else if ( strcmp( key, "MMMM" ) == 0 )
			{
				for ( size_t i = 0; i < 12; i++ )
				{
					if ( str_view.find( MMMM[i], k ) == k )
					{
						k += strlen( MMMM[i] );
						month = i + 1;
					}
				}
			}
			else if ( strcmp( key, "MMM" ) == 0 )
			{
				for ( size_t i = 0; i < 12; i++ )
				{
					if ( str_view.find( MMM[i], k ) == k )
					{
						k += strlen( MMM[i] );
						month = i + 1;
					}
				}
			}
			else if ( strcmp( key, "MM" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, month );
				k += 2;
			}
			else if ( strcmp( key, "M" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit( str_view[k + 1] ) )
					{
						std::from_chars( str_view.data() + k, str_view.data() + k + 2, month );
						k += 2;
					}
					else
					{
						month = str_view[k] - '0';
						k++;
					}
				}
			}
			else if ( strcmp( key, "dddd" ) == 0 )
			{
				for ( size_t i = 0; i < 12; i++ )
				{
					if ( str_view.find( dddd[i], k ) == k )
					{
						k += strlen( dddd[i] );
						week = i + 1;
					}
				}
			}
			else if ( strcmp( key, "ddd" ) == 0 )
			{
				for ( size_t i = 0; i < 12; i++ )
				{
					if ( str_view.find( ddd[i], k ) == k )
					{
						k += strlen( ddd[i] );
						week = i + 1;
					}
				}
			}
			else if ( strcmp( key, "dd" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, day );
				k += 2;
			}
			else if ( strcmp( key, "d" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit( str_view[k + 1] ) )
					{
						std::from_chars( str_view.data() + k, str_view.data() + k + 2, day );
						k += 2;
					}
					else
					{
						day = str_view[k] - '0';
						k++;
					}
				}
			}
			else if ( strcmp( key, "hh" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, hour );
				k += 2;
			}
			else if ( strcmp( key, "h" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit( str_view[k + 1] ) )
					{
						std::from_chars( str_view.data() + k, str_view.data() + k + 2, hour );
						k += 2;
					}
					else
					{
						hour = str_view[k] - '0';
						k++;
					}
				}
				}
			else if ( strcmp( key, "mm" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, minute );
				k += 2;
			}
			else if ( strcmp( key, "m" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit( str_view[k + 1] ) )
					{
						std::from_chars( str_view.data() + k, str_view.data() + k + 2, minute );
						k += 2;
					}
					else
					{
						minute = str_view[k] - '0';
						k++;
					}
				}
			}
			else if ( strcmp( key, "ss" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 2, second );
				k += 2;
			}
			else if ( strcmp( key, "s" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit( str_view[k + 1] ) )
					{
						std::from_chars( str_view.data() + k, str_view.data() + k + 2, second );
						k += 2;
					}
					else
					{
						second = str_view[k] - '0';
						k++;
					}
				}
			}
			else if ( strcmp( key, "zzz" ) == 0 )
			{
				std::from_chars( str_view.data() + k, str_view.data() + k + 3, millisecond );
				k += 3;
			}
			else if ( strcmp( key, "z" ) == 0 )
			{
				if ( ::isdigit( str_view[k] ) )
				{
					if ( k + 1 < str_view.size() && ::isdigit(str_view[k + 1]) )
					{
						if ( k + 2 < str_view.size() && ::isdigit( str_view[k+2] ) )
						{
							std::from_chars( str_view.data() + k, str_view.data() + k + 3, millisecond );
							k += 3;
						}
						else
						{
							std::from_chars( str_view.data() + k, str_view.data() + k + 2, millisecond );
							k += 2;
						}
					}
					else
					{
						millisecond = str_view[k] - '0';
						k++;
					}
				}
			}
			else
			{
				k += strlen( key );
			}
		}
	}
	
	return x::time_2_stamp( std::chrono::system_clock::time_point( std::chrono::years( year ) + std::chrono::months( month ) + std::chrono::days( day ) + std::chrono::hours( hour ) + std::chrono::minutes( minute ) + std::chrono::seconds( second ) + std::chrono::milliseconds( millisecond ) ) );
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

x_iconv x_iconv_create( x_string fromcode, x_string tocode )
{
	return iconv_open( tocode, fromcode );
}
uint64 x_iconv_translate( x_iconv iconv, x_string inbuf, uint64 inbytes, x_buffer outbuf )
{
	inbytes = std::min<uint64>( inbytes, strlen( inbuf ) );

	auto buf = (x::buffer *)outbuf;

	auto sz = ::iconv( iconv, &inbuf, &inbytes, nullptr, nullptr );

	auto out = buf->prepare( sz );

	sz = ::iconv( iconv, &inbuf, &inbytes, &out, &sz );

	buf->commit( sz );

	return sz;
}
void x_iconv_release( x_iconv iconv )
{
	iconv_close( iconv );
}

x_atomic x_atomic_create()
{
	return new std::atomic<x_atomic>( 0 );
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

x_buffer x_buffer_create()
{
	return new x::buffer;
}
int8 x_buffer_read_i8( x_buffer buffer )
{
	int8 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
int16 x_buffer_read_i16( x_buffer buffer )
{
	int16 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
int32 x_buffer_read_i32( x_buffer buffer )
{
	int32 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
int64 x_buffer_read_i64( x_buffer buffer )
{
	int64 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
uint8 x_buffer_read_u8( x_buffer buffer )
{
	uint8 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
uint16 x_buffer_read_u16( x_buffer buffer )
{
	uint16 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
uint32 x_buffer_read_u32( x_buffer buffer )
{
	uint32 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
uint64 x_buffer_read_u64( x_buffer buffer )
{
	uint64 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
float16 x_buffer_read_f16( x_buffer buffer )
{
	float16 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return x::endian::little_to_native( result );
}
float32 x_buffer_read_f32( x_buffer buffer )
{
	float32 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return result;
}
float64 x_buffer_read_f64( x_buffer buffer )
{
	float64 result = 0;
	std::istream( static_cast<x::buffer *>( buffer ) ).read( (char *)( &result ), sizeof( result ) );
	return result;
}
x_string x_buffer_read_str( x_buffer buffer )
{
	std::string str;
	std::istream is( static_cast<x::buffer *>( buffer ) );
	do
	{
		str.push_back( is.get() );
	} while ( str.back() != 0 );

	return x::allocator::salloc( str );
}
void x_buffer_write_i8( x_buffer buffer, int8 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_i16( x_buffer buffer, int16 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_i32( x_buffer buffer, int32 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_i64( x_buffer buffer, int64 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_u8( x_buffer buffer, uint8 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_u16( x_buffer buffer, uint16 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_u32( x_buffer buffer, uint32 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_u64( x_buffer buffer, uint64 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_f16( x_buffer buffer, float16 val )
{
	val = x::endian::native_to_little( val );
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_f32( x_buffer buffer, float32 val )
{
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_f64( x_buffer buffer, float64 val )
{
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( (const char *)( &val ), sizeof( val ) );
}
void x_buffer_write_str( x_buffer buffer, x_string val )
{
	std::ostream( static_cast<x::buffer *>( buffer ) ).write( val, strlen( val ) + 1 );
}
intptr x_buffer_prepare( x_buffer buffer, uint64 size )
{
	return static_cast<x::buffer *>( buffer )->prepare( size );
}
void x_buffer_commit( x_buffer buffer, uint64 size )
{
	static_cast<x::buffer *>( buffer )->commit( size );
}
void x_buffer_consume( x_buffer buffer, uint64 size )
{
	static_cast<x::buffer *>( buffer )->consume( size );
}
uint64 x_buffer_size( x_buffer buffer )
{
	return static_cast<x::buffer *>( buffer )->size();
}
intptr x_buffer_data( x_buffer buffer )
{
	return ( intptr )static_cast<x::buffer *>( buffer )->data();
}
int64 x_buffer_tellg( x_buffer buffer )
{
	return std::istream( static_cast<x::buffer *>( buffer ) ).tellg();
}
int64 x_buffer_tellp( x_buffer buffer )
{
	return std::ostream( static_cast<x::buffer *>( buffer ) ).tellp();
}
void x_buffer_seekg( x_buffer buffer, uint32 pos, int64 offset )
{
	switch ( pos )
	{
	case SEEK_POS_BEG:
		std::istream( static_cast<x::buffer *>( buffer ) ).seekg( offset, std::ios::beg );
		break;
	case SEEK_POS_CUR:
		std::istream( static_cast<x::buffer *>( buffer ) ).seekg( offset, std::ios::cur );
		break;
	case SEEK_POS_END:
		std::istream( static_cast<x::buffer *>( buffer ) ).seekg( offset, std::ios::end );
		break;
	default:
		break;
	}
}
void x_buffer_seekp( x_buffer buffer, uint32 pos, int64 offset )
{
	switch ( pos )
	{
	case SEEK_POS_BEG:
		std::ostream( static_cast<x::buffer *>( buffer ) ).seekp( offset, std::ios::beg );
		break;
	case SEEK_POS_CUR:
		std::ostream( static_cast<x::buffer *>( buffer ) ).seekp( offset, std::ios::cur );
		break;
	case SEEK_POS_END:
		std::ostream( static_cast<x::buffer *>( buffer ) ).seekp( offset, std::ios::end );
		break;
	default:
		break;
	}
}
void x_buffer_release( x_buffer buffer )
{
	delete static_cast<x::buffer *>( buffer );
}

x_uchardet x_uchardet_create()
{
	return uchardet_new();
}
int32 x_uchardet_handle_data( x_uchardet uchardet, x_string data, uint32 len )
{
	return uchardet_handle_data( (uchardet_t)uchardet, data, len );
}
void x_uchardet_data_end( x_uchardet uchardet )
{
	uchardet_data_end( (uchardet_t)uchardet );
}
x_string x_uchardet_get_charset( x_uchardet uchardet )
{
	return uchardet_get_charset( (uchardet_t)uchardet );
}
void x_uchardet_release( x_uchardet uchardet )
{
	uchardet_delete( (uchardet_t)uchardet );
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
