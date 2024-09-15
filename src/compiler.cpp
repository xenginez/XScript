#include "compiler.h"

#include <fstream>
#include <iostream>

#include "xlib.h"
#include "value.h"
#include "object.h"
#include "logger.h"
#include "module.h"
#include "grammar.h"
#include "builtin.h"
#include "symbols.h"
#include "codegen.h"
#include "semantic.h"
#include "exception.h"

namespace
{
	using any = x::value;
}

x::compiler::compiler( const x::logger_ptr & logger )
{
	_grammar = std::make_shared<x::grammar>();

	if ( logger )
		_logger = logger;
	else
		_logger = std::make_shared<x::logger>();
}

x::compiler::~compiler()
{
}

x::logger_ptr x::compiler::logger() const
{
	return _logger;
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
	_symbols = make_symbols();

	try
	{
		loading( file );

		scanner();

		analyzer();

		genmodules();

		linkmodule();
	}
	catch ( const std::exception & e )
	{
		_logger->error( e.what() );

		return false;
	}

	return true;
}

void x::compiler::scanner()
{
	x::symbol_scanner_visitor scanner;

	for ( const auto & it : _objects )
		scanner.scanner( _logger, _symbols, it->ast );
}

void x::compiler::analyzer()
{
	x::semantics_analyzer_visitor analyzer;

	for ( const auto & it : _objects )
		analyzer.analysis( _logger, _symbols, it->ast );
}

void x::compiler::loading( const x::url & url )
{
}

void x::compiler::loading( const std::filesystem::path & file )
{
	std::filesystem::path path;
	
	if ( std::filesystem::exists( file ) )
	{
		path = file;
	}
	else if ( file.is_relative() )
	{
		auto tmp = ( _relative_paths.back() / file );
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
				path = tmp;
				break;
			}
		}
	}

	path.make_preferred();

	XTHROW( x::compile_exception, path.empty(), "" );

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
		
		XTHROW( x::compile_exception, !ifs.is_open(), "" );

		obj->ast = x::grammar().parse( ifs, path.string() );

		logger()->info( std::format( "loading script {}", path.string() ) );
	}

	_relative_paths.push_back( path.parent_path() );
	for ( const auto & it : obj->ast->get_imports() )
	{
		loading( std::filesystem::path( it->get_path() ) );
	}
	_relative_paths.pop_back();
}

x::module_compiler::module_compiler( const x::logger_ptr & logger )
	: compiler( logger )
{
}

x::module_compiler::~module_compiler()
{
}

x::module_ptr x::module_compiler::module() const
{
	return _module;
}

void x::module_compiler::genmodules()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::module_compiler::object>( it );
		if ( obj->reset )
		{
			obj->reset = false;
			obj->module = std::make_shared<x::module>();

			x::module_scanner_visitor().scanner( obj->module, logger(), symbols(), obj->ast );

			x::module_generater_visitor().generate( obj->module, logger(), symbols(), obj->ast );

			logger()->info( std::format( "gen module: {}", obj->path.string() ) );
		}
	}
}

void x::module_compiler::linkmodule()
{
	_module = std::make_shared<x::module>();

	for ( auto & it : objects() )
	{
		_module->merge( std::static_pointer_cast<x::module_compiler::object>( it )->module );

		logger()->info( std::format( "link module: {}", it->path.string() ) );
	}
}

x::symbols_ptr x::module_compiler::make_symbols()
{
	auto symbols = std::make_shared<x::symbols>();

	//////////////////////////////////// foundation ////////////////////////////////////
#define BEG( TYPE ) auto TYPE##_type = symbols->add_foundation( #TYPE, sizeof( TYPE ) ); symbols->push_scope( TYPE##_type );
#define END() symbols->pop_scope();

	auto void_type = symbols->add_foundation( "void", 0 );
	
	BEG( any );
	END();
	BEG( bool );
	END();
	BEG( byte );
	END();
	BEG( int8 );
	END();
	BEG( int16 );
	END();
	BEG( int32 );
	END();
	BEG( int64 );
	END();
	BEG( uint8 );
	END();
	BEG( uint16 );
	END();
	BEG( uint32 );
	END();
	BEG( uint64 );
	END();
	BEG( float16 );
	END();
	BEG( float32 );
	END();
	BEG( float64 );
	END();
	BEG( intptr );
	END();
	BEG( string );
	END();
	BEG( object );
	END();
	BEG( coroutine );
	END();

#undef BEG
#undef END
	//////////////////////////////////// foundation ////////////////////////////////////


	//////////////////////////////////// native function ////////////////////////////////////
#define BEG( TYPE ) auto TYPE##_func = symbols->add_nativefunc( #TYPE, TYPE ); symbols->push_scope( TYPE##_func );
#define END() symbols->pop_scope();


	// path
	BEG( x_path_app_path );
	END();
	BEG( x_path_temp_path );
	END();
	BEG( x_path_copy );
	END();
	BEG( x_path_create );
	END();
	BEG( x_path_remove );
	END();
	BEG( x_path_exists );
	END();
	BEG( x_path_is_file );
	END();
	BEG( x_path_is_directory );
	END();
	BEG( x_path_entry_count );
	END();
	BEG( x_path_at_entry );
	END();


#undef BEG
#undef END
	//////////////////////////////////// native function ////////////////////////////////////


	//////////////////////////////////// builtin function ////////////////////////////////////
#define BEG( TYPE ) auto TYPE##_builtin = symbols->add_builtinfunc( #TYPE, std::make_shared<TYPE>() ); symbols->push_scope( TYPE##_builtin );
#define END() symbols->pop_scope();

	BEG( builtin_sizeof );
	END();
	BEG( builtin_typeof );
	END();
	BEG( builtin_list_sizeof );
	END();
	BEG( builtin_list_typeof );
	END();
	BEG( builtin_is_same );
	END();
	BEG( builtin_is_enum );
	END();
	BEG( builtin_is_class );
	END();
	BEG( builtin_is_base_of );
	END();
	BEG( builtin_is_template_of );
	END();

#undef BEG
#undef END
	//////////////////////////////////// builtin function ////////////////////////////////////

	return symbols;
}

x::compiler::object_ptr x::module_compiler::make_object()
{
	return std::make_shared<x::module_compiler::object>();
}

x::llvm_compiler::llvm_compiler( const x::logger_ptr & logger )
	: compiler( logger )
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

void x::llvm_compiler::genmodules()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::llvm_compiler::object>( it );
		if ( obj->reset )
		{
			obj->reset = false;
			//obj->module = std::make_shared<llvm::module>( obj->path.string(), *_context );

			x::llvm_scanner_visitor().scanner( obj->module, logger(), symbols(), obj->ast );

			x::llvm_generater_visitor().generate( obj->module, logger(), symbols(), obj->ast );
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

x::symbols_ptr x::llvm_compiler::make_symbols()
{
	auto sym = std::make_shared<x::symbols>();

	return sym;
}

x::compiler::object_ptr x::llvm_compiler::make_object()
{
	return std::make_shared<x::llvm_compiler::object>();
}

x::spirv_compiler::spirv_compiler( const x::logger_ptr & logger )
	: compiler( logger )
{
}

x::spirv_compiler::~spirv_compiler()
{
}

spirv::module_ptr x::spirv_compiler::module() const
{
	return _module;
}

void x::spirv_compiler::genmodules()
{
	for ( auto & it : objects() )
	{
		auto obj = std::static_pointer_cast<x::spirv_compiler::object>( it );
		if ( obj->reset )
		{
			obj->reset = false;
			//obj->module = std::make_shared<spirv::module>( obj->path.string(), *_context );

			x::spirv_scanner_visitor().scanner( obj->module, logger(), symbols(), obj->ast );

			x::spirv_generater_visitor().generate( obj->module, logger(), symbols(), obj->ast );
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

x::symbols_ptr x::spirv_compiler::make_symbols()
{
	auto sym = std::make_shared<x::symbols>();

	return sym;
}

x::compiler::object_ptr x::spirv_compiler::make_object()
{
	return std::make_shared<x::spirv_compiler::object>();
}
