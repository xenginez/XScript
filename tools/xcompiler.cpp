#include <thread>

#include <xlib.h>
#include <json.hpp>
#include <logger.h>
#include <compiler.h>
#include <scheduler.h>

namespace
{
	struct project
	{
		std::string archive = "x";
		std::string type = "module";
		std::string mode = "debug|release";
		std::string name = "";
		std::string auther;
		std::string origin;
		std::string version;
		std::string entry;
		std::string srcpath;
		std::string install;
		std::vector<std::string> depends;
		std::vector<std::string> linkpaths;

		x::uint64 priority = 0;
		x::compiler_ptr compiler;
	};

	x::logger _logger;
	std::vector<project> _projects;

	x::uint32 version_str_to_num( std::string_view str )
	{
		x::uint32 v1 = 0, v2 = 0, v3 = 0, v4 = 0;

		auto beg = str.data();
		auto end = str.data() + str.size();
		auto cur = str.data();
		
		for ( ; *cur != '.' && cur <= end; ++cur );
		std::from_chars( beg, cur, v1 ); ++cur;
		for ( beg = cur; *cur != '.' && cur <= end; ++cur );
		std::from_chars( beg, cur, v2 ); ++cur;
		for ( beg = cur; *cur != '.' && cur <= end; ++cur );
		std::from_chars( beg, cur, v3 ); ++cur;
		for ( beg = cur; *cur != '.' && cur <= end; ++cur );
		std::from_chars( beg, cur, v4 ); ++cur;

		return ( ( v1 << 24 ) | ( v2 << 16 ) | ( v3 << 8 ) | v4 );
	}
}

bool scanner_project()
{
	auto xmake = std::filesystem::current_path() / "xmake.json";

	if ( !std::filesystem::exists( xmake ) )
	{
		_logger.error( std::format( "\'xmake.json\' does not exist in the \'{}\'!", std::filesystem::current_path().string() ) );
		return false;
	}

	auto json = x::json::load( xmake );

	for ( const auto & it : json.to_object() )
	{
		project proj;
		const auto & json = it.second;

#define CHECKER( KEY ) \
		if ( json.contains( #KEY ) ) \
		{ \
			_logger.error( std::format( "project \'{}\' does not fill in the \'{}\'!", it.first, #KEY ) ); \
			return false; \
		} \
		else \
		{ \
			proj.name = json[#KEY]; \
		}

		CHECKER( name );
		CHECKER( version );
		CHECKER( archive );
		CHECKER( srcpath );
#undef CHECKER

		if ( json.contains( "type" ) ) proj.type = json["type"];
		if ( json.contains( "mode" ) ) proj.mode = json["mode"];
		if ( json.contains( "auther" ) ) proj.auther = json["auther"];
		if ( json.contains( "origin" ) ) proj.origin = json["origin"];
		if ( json.contains( "entry" ) ) proj.entry = json["entry"];
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

	return !_projects.empty();
}
bool setting_compiler()
{
	for ( auto & it : _projects )
	{
		if ( it.archive == "x" )
			it.compiler = std::make_shared<x::compiler>();
		else if ( it.archive == "llvm" )
			it.compiler = std::make_shared<x::llvm_compiler>();
		else if ( it.archive == "spirv" )
			it.compiler = std::make_shared<x::spirv_compiler>();

		it.compiler->set_module_name( it.name );
		it.compiler->set_module_author( it.auther );
		it.compiler->set_module_origin( it.origin );
		it.compiler->set_module_version( version_str_to_num( it.version ) );

		for ( const auto & iter : it.linkpaths )
		{
			it.compiler->add_link_path( iter );
		}
	}

	return true;
}
bool priority_project()
{
	for ( auto & it : _projects )
	{
		for ( const auto & iter : it.depends )
		{
			auto it2 = std::find_if( _projects.begin(), _projects.end(), [&]( const auto & val ) { return val.name == iter; } );
			if ( it2 != _projects.end() )
			{
				it2->priority++;
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}
x::awaitable<bool> compile_project( project * pro )
{
	co_await x::scheduler::transfer_work();

	if ( pro->compiler->compile( pro->srcpath ) )
	{

		co_return true;
	}


	co_return false;
}
x::awaitable<void> taskgraph_compile()
{
	for ( size_t i = _projects.size() - 1; i >= 0; --i )
	{
		for ( auto & it : _projects )
		{
			if ( it.priority == i )
			{
				if ( !( co_await compile_project( &it ) ) )
				{
					co_return;
				}
			}
		}
	}
}

int main( int argc, char * argv[] )
{
	if ( scanner_project() )
	{
		if ( setting_compiler() )
		{
			taskgraph_compile().detach();
		}
	}

	x::scheduler::init();

	while ( x::scheduler::run() );

	return 0;
}
