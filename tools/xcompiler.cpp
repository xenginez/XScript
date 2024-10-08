#include <thread>

#include <xlib.h>
#include <json.hpp>
#include <compiler.h>
#include <scheduler.h>

namespace
{
	struct project
	{
		std::string archive = "x";
		std::string type = "module";
		std::string mode = "debug";
		std::string name = "";
		std::string auther;
		std::string origin;
		std::string version;
		std::string entry;
		std::string srcpath;
		std::string install;
		std::vector<std::string> depends;
		std::vector<std::string> linkpaths;
	};

	std::vector<project> _projects;
}

void scanner_project()
{
	auto xmake = std::filesystem::current_path() / "xmake.json";

	if ( !std::filesystem::exists( xmake ) )
	{
		std::cout << std::format( "[xcompiler] error: \'xmake.json\' does not exist in the {}", std::filesystem::current_path().string() ) << std::endl;
		return;
	}

	auto json = x::json::load( xmake );
	std::string timenow = x_time_to_string( x_time_now(), "yyyyMMddhhmmss" );

	for ( const auto & it : json.to_object() )
	{
		project proj;
		const auto & json = it.second;

		if ( json.contains( "archive" ) ) proj.archive = json["archive"];
		if ( json.contains( "type" ) ) proj.type = json["type"];
		if ( json.contains( "mode" ) ) proj.mode = json["mode"];
		if ( json.contains( "name" ) ) proj.name = json["name"];
		if ( json.contains( "auther" ) ) proj.auther = json["auther"];
		if ( json.contains( "origin" ) ) proj.origin = json["origin"];
		if ( json.contains( "entry" ) ) proj.entry = json["entry"];
		if ( json.contains( "srcpath" ) ) proj.srcpath = json["srcpath"];

		if ( json.contains( "version" ) ) proj.version = json["version"]; else proj.version = timenow;
		if ( json.contains( "install" ) ) proj.install = json["install"]; else proj.install = std::filesystem::current_path().string();

		if ( json.contains( "depends" ) )
		{
			auto depends = json["depends"].to_array();
			for ( const auto & it : depends )
				proj.depends.push_back( it );
		}
		if ( json.contains( "linkpaths" ) )
		{
			auto linkpaths = json["linkpaths"].to_array();
			for ( const auto & it : linkpaths )
				proj.linkpaths.push_back( it );
		}

		std::cout << "project: " << it.first << std::endl;

		_projects.push_back( proj );
	}
}
bool setting_compiler( x::compiler * comp, project * proj )
{

}
x::awaitable<void> task_compile( int i )
{
	printf( std::format( "{} - {}: main {}\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ), i, x_time_to_string( x_time_now(), "yyyy-MM-dd hh:mm:ss.zzz" ) ).c_str() );

	co_await x::scheduler::instance()->sleep_for( std::chrono::seconds( 1 ) );

	printf( std::format( "{} - {}: time {}\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ), i, x_time_to_string( x_time_now(), "yyyy-MM-dd hh:mm:ss.zzz" ) ).c_str() );

	co_await x::scheduler::instance()->sleep_for( std::chrono::seconds( 1 ) );

	co_await x::scheduler::instance()->transfer_main();
	printf( std::format( "{} - {}: main {}\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ), i, x_time_to_string( x_time_now(), "yyyy-MM-dd hh:mm:ss.zzz" ) ).c_str() );

	for ( size_t i = 0; i < std::thread::hardware_concurrency(); i++ )
	{
		co_await x::scheduler::instance()->transfer_work();
		printf( std::format( "{} - {}: work {}\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ), i, x_time_to_string( x_time_now(), "yyyy-MM-dd hh:mm:ss.zzz" ) ).c_str() );
	}

	x::scheduler::instance()->shutdown();

	co_return;
}

int main( int argc, char * argv[] )
{
	printf( "main thread: %d\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ) );

	x::scheduler::instance()->init();

	task_compile( 0 ).detach();

	while ( x::scheduler::instance()->run() );

	scanner_project();
	if ( !_projects.empty() )
	{
	
	}

	return 0;
}
