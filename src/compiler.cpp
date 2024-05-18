#include "compiler.h"

#include <fstream>
#include <iostream>

#include "module.h"
#include "grammar.h"
#include "symbols.h"
#include "visitor.h"
#include "exception.h"

x::compiler::compiler( const log_callback_t & callback )
{
	if ( callback )
		_logger = callback;
	else
		_logger = []( std::string_view message ) { std::cout << message << std::endl; };
}

x::compiler::~compiler()
{
}

x::symbols_ptr x::compiler::symbols() const
{
	return _symbols;
}

std::span<const x::compiler::object_ptr> x::compiler::objects() const
{
	return _objects;
}

void x::compiler::add_search_path( const std::filesystem::path & path )
{
	if ( std::find( _absolute_paths.begin(), _absolute_paths.end(), path ) == _absolute_paths.end() )
		_absolute_paths.push_back( path );
}

bool x::compiler::compile( const std::filesystem::path & file )
{
	_symbols = std::make_shared<x::scanner>();

	try
	{
		load_source_file( file );

		scanner();

		instant();

		checker();

		genunit();

		linking();

	}
	catch ( const std::exception & e )
	{
		_logger( e.what() );

		return false;
	}

	return true;
}

void x::compiler::scanner()
{
	x::symbol_scanner_visitor scaner( _symbols );

	for ( const auto & it : _objects )
		it->ast->accept( &scaner );
}

void x::compiler::instant()
{
	//x::instant_template_pass instant( _symbols );
	//
	//for ( const auto & it : _objects )
	//	it->ast->accept( &instant );
}

void x::compiler::checker()
{
	x::semantic_checker_visitor semantic( _symbols );

	for ( const auto & it : _objects )
		it->ast->accept( &semantic );
}

void x::compiler::genunit()
{
	genmodule();

	_logger( std::format( "gen object success" ) );
}

void x::compiler::linking()
{
	genmodule();

	_logger( std::format( "link all module success" ) );
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
		auto tmp = ( _relative_paths.back() / file ).make_preferred();
		if ( std::filesystem::exists( tmp ) )
			path = tmp;
	}
	else
	{
		for ( const auto & it : _absolute_paths )
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

	object_ptr obj = nullptr;
	auto time = std::filesystem::last_write_time( path );

	auto it = std::find_if( _objects.begin(), _objects.end(), [path]( const auto & val ) { return val->path == path; } );
	if ( it != _objects.end() )
	{
		if ( time != (*it)->time )
		{
			(*it)->ast = nullptr;
			(*it)->module = nullptr;
		}

		obj = *it;
	}
	else
	{
		obj = make_object();
		obj->path = path;
		_objects.push_back( obj );
	}

	obj->time = time;

	if ( obj->ast == nullptr )
	{
		std::ifstream ifs( path );
		
		ASSERT( ifs.is_open(), "" );

		obj->ast = x::grammar( ifs, path.string() ).unit();

		_logger( std::format( "loading script {}", path.string() ) );
	}

	_relative_paths.push_back( path.parent_path() );
	for ( const auto & it : obj->ast->imports )
	{
		load_source_file( it->path );
	}
	_relative_paths.pop_back();
}

x::module_compiler::module_compiler( const log_callback_t & callback )
	: compiler( callback )
{
}

x::module_compiler::~module_compiler()
{
}

x::module_ptr x::module_compiler::module() const
{
	return _module;
}

void x::module_compiler::genmodule()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::module_compiler::object>( it );
		if ( obj->module == nullptr )
		{
			obj->module = std::make_shared<x::module>();
			obj->module->generate( symbols(), obj->ast );
		}
	}
}

void x::module_compiler::linkmodule()
{
	_module = std::make_shared<x::module>();

	for ( auto & it : objects() )
	{
		_module->merge( std::static_pointer_cast<x::module_compiler::object>( it )->module );
	}
}

x::compiler::object_ptr x::module_compiler::make_object()
{
	return std::make_shared<x::module_compiler::object>();
}

x::llvmir_compiler::llvmir_compiler( const log_callback_t & callback )
	: compiler( callback )
{
	_context = nullptr;
	_module = nullptr;
}

x::llvmir_compiler::~llvmir_compiler()
{
	// if ( _module ) delete _module;
	// if ( _context ) delete _context;
}

llvm::module_ptr x::llvmir_compiler::module() const
{
	return _module;
}

void x::llvmir_compiler::genmodule()
{
}

void x::llvmir_compiler::linkmodule()
{
}

x::compiler::object_ptr x::llvmir_compiler::make_object()
{
	auto obj = std::make_shared<x::llvmir_compiler::object>();
	return obj;
}
