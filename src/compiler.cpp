#include "compiler.h"

#include <map>
#include <fstream>

#include "pass.h"
#include "module.h"
#include "grammar.h"
#include "symbols.h"
#include "context.h"

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
	std::vector<std::filesystem::path> _paths;
	std::map<std::filesystem::path, uobject> _uobjs;
};

x::compiler::compiler()
	: _p( new private_p )
{
}

x::compiler::~compiler()
{
	delete _p;
}

void x::compiler::add_search_path( const std::filesystem::path & path )
{
	_p->_paths.push_back( path );
}

x::context_ptr x::compiler::compile( const std::filesystem::path & file )
{
	_p->_symbols = std::make_shared<x::symbols>();

	if ( load_source_file( file ) )
	{
		symbol_scanner_pass scaner( _p->_symbols );
		type_checker_pass checker( _p->_symbols );
		semantic_checker_pass semantic( _p->_symbols );

		for ( const auto & it : _p->_uobjs ) it.second.ast->accept( &scaner );
		for ( const auto & it : _p->_uobjs ) it.second.ast->accept( &checker );
		for ( const auto & it : _p->_uobjs ) it.second.ast->accept( &semantic );

		for ( auto & it : _p->_uobjs )
		{
			if ( it.second.module == nullptr )
			{
				it.second.module = std::make_shared<x::module>();
				it.second.module->generate( _p->_symbols, it.second.ast );
			}
		}

		return generate();
	}

	return {};
}

bool x::compiler::load_source_file( std::filesystem::path file )
{
	bool result = false;
	std::filesystem::path path;
	
	if ( std::filesystem::exists( file ) )
	{
		path = file.make_preferred();
	}
	else
	{
		for ( auto it = _p->_paths.rbegin(); it != _p->_paths.rend(); ++it )
		{
			auto tmp = ( *it / path ).make_preferred();
			if ( std::filesystem::exists( tmp ) )
			{
				path = tmp;
				break;
			}
		}
	}

	if ( path.empty() ) return result;

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
		
		if ( !ifs.is_open() ) return result;

		u->ast = x::grammar( ifs, path.string() ).unit();
	}

	result = true;
	
	_p->_paths.push_back( path.parent_path() );
	for ( const auto & it : u->ast->imports )
	{
		if ( !load_source_file( it->path ) )
		{
			result = false;
			break;
		}
	}
	_p->_paths.pop_back();

	return result;
}

x::context_ptr x::compiler::generate() const
{
	auto ctx = std::make_shared<x::context>();



	return ctx;
}
