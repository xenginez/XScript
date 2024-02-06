#pragma once

#include <span>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <vector>
#include <numbers>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <shared_mutex>

#include "type.h"

namespace x
{
	enum seek_dir
	{
		beg,
		cur,
		end,
	};
	enum open_mode
	{
		in = 0x01,
		out = 0x02,
		ate = 0x04,
		app = 0x08,
		trunc = 0x10,
		binary = 0x20,
	};
	enum copy_options
	{
		skip_existing = 0x1,
		overwrite_existing = 0x2,
		update_existing = 0x4,
		directories_only = 0x1000,
		recursive = 0x10,
	};
	enum directory_options
	{
		files = 1 << 0,
		directorys = 1 << 2,
		dot_and_dotdot = 1 << 3,
		all_entrys = files | directorys,
	};

	class path
	{
	public:
		path() = default;
		path( const path & val );
		path( std::string_view val );
		path( const std::filesystem::path & val );

	public:
		void clear();
		path & append( std::string_view src );
		path & concat( std::string_view src );
		int compare( const path & src ) const;
		std::string string() const;
		path filename() const;
		path stem() const;
		path extension() const;
		bool empty() const;

	public:
		bool exists() const;
		bool is_file() const;
		bool is_directory() const;
		int64_t file_size() const;
		bool remove() const;
		bool remove_all() const;
		void rename( const path & npath ) const;
		void resize( int64_t size ) const;
		bool create() const;
		void copy( const path & val, copy_options opt ) const;
		std::vector<path> entrys( directory_options opt ) const;

	public:
		static path current_path();
		static path temp_directory_path();

	private:
		std::filesystem::path _path;
	};
	class file
	{
	public:
		file() = default;
		file( const file & val ) = delete;
		file( const path & val, int mode = open_mode::in | open_mode::out );

	public:
		bool is_open() const;

	public:
		void open( const path & val, int mode = open_mode::in | open_mode::out );
		void close();
		void flush();

	public:
		int64_t get();
		int64_t geti8();
		int64_t geti16();
		int64_t geti32();
		int64_t geti64();
		int64_t read( std::span<uint8_t> data, int64_t off = 0, int64_t size = -1 );
		int64_t tellg();
		void seekg( int64_t off, seek_dir dir = seek_dir::cur );

	public:
		void put( int64_t c );
		void puti8( int64_t c );
		void puti16( int64_t c );
		void puti32( int64_t c );
		void puti64( int64_t c );
		void write( std::span<uint8_t> data, int64_t size = -1 );
		int64_t tellp();
		void seekp( int64_t off, seek_dir dir = seek_dir::cur );

	private:
		std::fstream _file;
	};
	class time
	{
		using clock_type = std::chrono::system_clock;

	public:
		static x::time now();

	public:
		int64_t year() const;
		int64_t month() const;
		int64_t day() const;
		int64_t hour() const;
		int64_t minute() const;
		int64_t second() const;
		int64_t millisecond() const;

	public:
		void set_year( int64_t val );
		void set_month( int64_t val );
		void set_day( int64_t val );
		void set_hour( int64_t val );
		void set_minute( int64_t val );
		void set_second( int64_t val );
		void set_millisecond( int64_t val );

	public:
		void add_year( int64_t val );
		void add_month( int64_t val );
		void add_day( int64_t val );
		void add_hour( int64_t val );
		void add_minute( int64_t val );
		void add_second( int64_t val );
		void add_millisecond( int64_t val );

	public:
		int64_t relative_year( time val ) const;
		int64_t relative_month( time val ) const;
		int64_t relative_day( time val ) const;
		int64_t relative_hour( time val ) const;
		int64_t relative_minute( time val ) const;
		int64_t relative_second( time val ) const;
		int64_t relative_millisecond( time val ) const;

	public:
		clock_type::time_point time_point() const;

	private:
		clock_type::time_point _time;
	};
	class buff
	{
	public:
		int64_t geti8();
		int64_t geti16();
		int64_t geti32();
		int64_t geti64();
		int64_t read( std::span<uint8_t> data, int64_t off = 0, int64_t size = -1 );
		int64_t tellg();
		void seekg( int64_t off, seek_dir dir = seek_dir::cur );

	public:
		void puti8( int64_t c );
		void puti16( int64_t c );
		void puti32( int64_t c );
		void puti64( int64_t c );
		void write( std::span<uint8_t> data, int64_t size = -1 );
		int64_t tellp();
		void seekp( int64_t off, seek_dir dir = seek_dir::cur );

	private:
		std::stringstream _stream;
	};
	class mutex
	{
	public:
		void lock();
		bool try_lock();
		void unlock();

	private:
		std::mutex _lock;
	};
	class rwmutex
	{
	public:
		void rlock();
		bool try_rlock();
		void runlock();

	public:
		void wlock();
		bool try_wlock();
		void wunlock();

	private:
		std::shared_mutex _lock;
	};
	class random
	{
	public:
		static int64_t device();

	public:
		void seed( int64_t val );

	public:
		int64_t irand( int64_t min, int64_t max );
		double frand( double min, double max );
		double normal_frand( double min, double max );

	private:
		std::default_random_engine _engine;
	};
	class thread
	{
	public:
		static int64_t get_id();
		static void sleep_for( int64_t msec );
		static void sleep_until( time val );
		static bool stop_requested();
		static int64_t hardware_concurrency();

	private:
		static std::stop_token & stop_token();

	public:
		void run();

	public:
		void join();
		void detach();
		bool request_stop();

	public:
		int64_t id() const;
		bool joinable() const;

	private:
		std::jthread _thread;
	};

	namespace math
	{
		static inline constexpr double e()
		{
			return std::numbers::e;
		}
		static inline constexpr double log2e()
		{
			return std::numbers::log2e;
		}
		static inline constexpr double log10e()
		{
			return std::numbers::log10e;
		}
		static inline constexpr double pi()
		{
			return std::numbers::pi;
		}
		static inline constexpr double inv_pi()
		{
			return std::numbers::inv_pi;
		}
		static inline constexpr double inv_sqrtpi()
		{
			return std::numbers::inv_sqrtpi;
		}
		static inline constexpr double ln2()
		{
			return std::numbers::ln2;
		}
		static inline constexpr double ln10()
		{
			return std::numbers::ln10;
		}
		static inline constexpr double sqrt2()
		{
			return std::numbers::sqrt2;
		}
		static inline constexpr double sqrt3()
		{
			return std::numbers::sqrt3;
		}
		static inline constexpr double inv_sqrt3()
		{
			return std::numbers::inv_sqrt3;
		}
		static inline constexpr double egamma()
		{
			return std::numbers::egamma;
		}
		static inline constexpr double phi()
		{
			return std::numbers::phi;
		}

	};
	namespace network
	{
		class tcp_client;
		class tcp_server;
		class tcp_session;
		class udp_client;
		class udp_server;
		class udp_session;
		class http_client;
		class http_server;
		class http_session;
	}

	static inline void printf( std::string_view fmt, std::span<x::value> args );
	static inline std::string format( std::string_view fmt, std::span<x::value> args );

	extern void load_stdlib( const x::context_ptr & ctx );
}
