#include "compiler.h"

#include <map>
#include <fstream>
#include <iostream>

#include "module.h"
#include "visitor.h"
#include "grammar.h"
#include "symbols.h"
#include "context.h"
#include "exception.h"

namespace
{
	struct uobject
	{
		x::unit_ast_ptr ast;
		x::module_ptr module;
		std::filesystem::path path;
		std::filesystem::file_time_type time;
	};
}

struct x::compiler::private_p
{
	x::symbols_ptr _symbols;
	x::compiler::log_callback_t _logger;
	std::map<std::filesystem::path, uobject> _uobjs;
	std::deque<std::filesystem::path> _relative_paths;
	std::vector<std::filesystem::path> _absolute_paths;
};

x::compiler::compiler( const log_callback_t & callback )
	: _p( new private_p )
{
	if ( callback )
		_p->_logger = callback;
	else
		_p->_logger = []( std::string_view message ) { std::cout << message << std::endl; };
}

x::compiler::~compiler()
{
	delete _p;
}

void x::compiler::add_search_path( const std::filesystem::path & path )
{
	_p->_absolute_paths.push_back( path );
}

x::module_ptr x::compiler::compile( const std::filesystem::path & file )
{
	_p->_symbols = std::make_shared<x::symbols>();

	try
	{
		load_source_file( file );

		symbols();

		instant();

		checker();

		gen_unit_module();

		return merge_all_module();
	}
	catch ( const std::exception & e )
	{
		_p->_logger( e.what() );
	}

	return nullptr;
}

void x::compiler::symbols()
{
	x::symbol_scanner_visitor scaner( _p->_symbols );

	for ( const auto & it : _p->_uobjs )
		it.second.ast->accept( &scaner );
}

void x::compiler::instant()
{
	//x::instant_template_pass instant( _p->_symbols );
	//
	//for ( const auto & it : _p->_uobjs )
	//	it.second.ast->accept( &instant );
}

void x::compiler::checker()
{
	x::semantic_checker_visitor semantic( _p->_symbols );

	for ( const auto & it : _p->_uobjs )
		it.second.ast->accept( &semantic );
}

void x::compiler::gen_unit_module()
{
	for ( auto & it : _p->_uobjs )
	{
		if ( it.second.module == nullptr )
		{
			it.second.module = std::make_shared<x::module>();
			it.second.module->generate( _p->_symbols, it.second.ast );
			_p->_logger( std::format( "building module {}", it.second.path.string() ) );
		}
	}
}

x::module_ptr x::compiler::merge_all_module()
{
	auto result = std::make_shared<x::module>();
	for ( auto & it : _p->_uobjs )
	{
		result->merge( it.second.module );
	}

	_p->_logger( std::format( "merging all module success" ) );

	return result;
}

void x::compiler::load_source_file( std::filesystem::path file )
{
	std::filesystem::path path;
	
	if ( std::filesystem::exists( file ) )
	{
		path = file.make_preferred();
	}
	else if ( file.is_relative() )
	{
		auto tmp = ( _p->_relative_paths.back() / file ).make_preferred();
		if ( std::filesystem::exists( tmp ) )
			path = tmp;
	}
	else
	{
		for ( const auto & it : _p->_absolute_paths )
		{
			auto tmp = it / file;
			if ( std::filesystem::exists( tmp ) )
			{
				path = tmp.make_preferred();
				break;
			}
		}
	}

	ASSERT( path.empty(), "" );

	uobject * u = nullptr;
	auto time = std::filesystem::last_write_time( path );

	auto it = _p->_uobjs.find( path );
	if ( it != _p->_uobjs.end() )
	{
		if ( time != it->second.time )
		{
			it->second.ast = nullptr;
			it->second.module = nullptr;
		}

		u = &it->second;
	}
	else
	{
		uobject tmp;
		tmp.path = path;
		u = &_p->_uobjs.emplace( path, tmp ).first->second;
	}

	u->time = time;

	if ( u->ast == nullptr )
	{
		std::ifstream ifs( path );
		
		ASSERT( ifs.is_open(), "" );

		u->ast = x::grammar( ifs, path.string() ).unit();

		_p->_logger( std::format( "loading script {}", path.string() ) );
	}

	_p->_relative_paths.push_back( path.parent_path() );
	for ( const auto & it : u->ast->imports )
	{
		load_source_file( it->path );
	}
	_p->_relative_paths.pop_back();
}
