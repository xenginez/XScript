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
	x::instantiate_visitor instant( _symbols );
	
	for ( const auto & it : _objects )
		it->ast->accept( &instant );
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

	for ( auto & it : _objects )
		it->reset = false;

	_logger( std::format( "gen object success" ) );
}

void x::compiler::linking()
{
	linkmodule();

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
			( *it )->reset = true;

		obj = *it;
	}
	else
	{
		obj = make_object();
		obj->path = path;
		obj->reset = true;
		_objects.push_back( obj );
	}

	obj->time = time;

	if ( obj->reset )
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

			x::module_scanner_visitor scanner( obj->module, symbols() );
			obj->ast->accept( &scanner );

			x::module_generater_visitor generater( obj->module, symbols() );
			obj->ast->accept( &generater );
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

x::llvm_compiler::llvm_compiler( const log_callback_t & callback )
	: compiler( callback )
{
	// _context = std::make_shared<llvm::context>();
}

x::llvm_compiler::~llvm_compiler()
{
	// if ( _context ) delete _context;
}

llvm::module_ptr x::llvm_compiler::module() const
{
	return _module;
}

void x::llvm_compiler::genmodule()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::llvm_compiler::object>( it );
		if ( obj->reset )
		{
			//obj->module = std::make_shared<llvm::module>( obj->path.string(), *_context );

			x::llvm_scanner_visitor scanner( obj->module, symbols() );
			obj->ast->accept( &scanner );

			x::llvm_generater_visitor generater( obj->module, symbols() );
			obj->ast->accept( &generater );
		}
	}
}

void x::llvm_compiler::linkmodule()
{
	// _module = std::make_shared<llvm::module>( "", *_context );

	for ( auto & it : objects() )
	{
		// llvm::Linker::linkModules( *_module, *it->module );
	}
}

x::compiler::object_ptr x::llvm_compiler::make_object()
{
	return std::make_shared<x::llvm_compiler::object>();
}

x::spirv_compiler::spirv_compiler( const log_callback_t & callback )
	: compiler( callback )
{
	// _context = std::make_shared<spirv::context>();
}

x::spirv_compiler::~spirv_compiler()
{
	// if ( _context ) delete _context;
}

spirv::module_ptr x::spirv_compiler::module() const
{
	return _module;
}

void x::spirv_compiler::genmodule()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::spirv_compiler::object>( it );
		if ( obj->reset )
		{
			//obj->module = std::make_shared<spirv::module>( obj->path.string(), *_context );

			x::spirv_scanner_visitor scanner( obj->module, symbols() );
			obj->ast->accept( &scanner );

			x::spirv_generater_visitor generater( obj->module, symbols() );
			obj->ast->accept( &generater );
		}
	}
}

void x::spirv_compiler::linkmodule()
{
	// _module = std::make_shared<spirv::module>( "", *_context );

	for ( auto & it : objects() )
	{
		// spirv::Linker::linkModules( *_module, *it->module );
	}
}

x::compiler::object_ptr x::spirv_compiler::make_object()
{
	return std::make_shared<x::spirv_compiler::object>();
}
