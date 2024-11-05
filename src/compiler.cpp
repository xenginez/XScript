#include "compiler.h"

#include <fstream>
#include <iostream>

#include "xlib.h"
#include "value.h"
#include "buffer.h"
#include "object.h"
#include "logger.h"
#include "module.h"
#include "grammar.h"
#include "builtin.h"
#include "symbols.h"
#include "codegen.h"
#include "semantic.h"
#include "optimize.h"
#include "exception.h"

namespace
{
	using any = x::value;
}

x::compiler::compiler( const x::logger_ptr & logger )
{
	if ( logger )
		_logger = logger;
	else
		_logger = std::make_shared<x::logger>();
}

x::compiler::~compiler()
{
}

void x::compiler::reload( const std::filesystem::path & file )
{
	XTHROW( x::compile_exception, !std::filesystem::exists( file ), "" );

	object_ptr obj = nullptr;
	auto time = std::filesystem::last_write_time( file );

	auto it = std::find_if( _objects.begin(), _objects.end(), [file]( const auto & val ) { return val->path == file; } );
	if ( it != _objects.end() )
	{
		if ( time != (*it)->time )
			( *it )->reset = true;

		obj = *it;
	}
	else
	{
		obj = std::make_shared<x::compiler::object>();
		obj->path = file;
		obj->reset = true;
		_objects.push_back( obj );
	}

	obj->time = time;

	if ( obj->reset )
	{
		std::ifstream ifs( file );
		
		XTHROW( x::compile_exception, !ifs.is_open(), "" );

		obj->ast = x::grammar().parse( ifs, file.string() );

		logger()->info( std::format( "load file: {}", file.string() ) );

		for ( const auto & it : obj->ast->get_imports() )
		{
			load_module( it->get_modulename() );
		}
	}
}

bool x::compiler::compile( const std::filesystem::path & path )
{
	_symbols = make_symbols();

	try
	{
		if ( std::filesystem::is_directory( path ) )
		{
			for ( std::filesystem::recursive_directory_iterator it( path ), end; it != end; ++it )
			{
				if ( !it->is_directory() )
				{
					auto ext = it->path().extension().string();
					std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );

					if ( ext == x::source_extension )
						reload( it->path() );
				}
			}
		}
		else
		{
			auto ext = path.extension().string();
			std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );

			XTHROW( x::compile_exception, ext != x::source_extension, "" );

			reload( path );
		}

		scanning();

		analysis();

		optimize();

		generate();

		linkmodule();

		compilemodule();

		logger()->info("compile success!" );
	}
	catch ( const std::exception & e )
	{
		_logger->error( e.what() );

		logger()->error( "compile fail!" );

		return false;
	}

	return true;
}

x::module_ptr x::compiler::module() const
{
	return _module;
}

x::logger_ptr x::compiler::logger() const
{
	return _logger;
}

x::symbols_ptr x::compiler::symbols() const
{
	return _symbols;
}

x::uint32 x::compiler::get_module_version() const
{
	return _version;
}

void x::compiler::set_module_version( x::uint32 version )
{
	_version = version;
}

const std::string & x::compiler::get_module_name() const
{
	return _name;
}

void x::compiler::set_module_name( const std::string & name )
{
	_name = name;
}

const std::string & x::compiler::get_module_author() const
{
	return _author;
}

void x::compiler::set_module_author( const std::string & author )
{
	_author = author;
}

const std::string & x::compiler::get_module_origin() const
{
	return _origin;
}

void x::compiler::set_module_origin( const std::string & origin )
{
	_origin = origin;
}

void x::compiler::add_link_path( const std::filesystem::path & path )
{
	_linkpaths.push_back( path );
}

void x::compiler::scanning()
{
	x::symbol_scan_visitor scan;

	for ( const auto & it : _objects )
		scan.scan( _logger, _symbols, it->ast );
}

void x::compiler::analysis()
{
	x::semantics_analysis_visitor analysis;

	for ( const auto & it : _objects )
		analysis.analysis( _logger, _symbols, it->ast );
}

void x::compiler::optimize()
{
	x::code_optimize_visitor optimize;

	for ( const auto & it : _objects )
		optimize.optimize( _logger, _symbols, it->ast );
}

void x::compiler::generate()
{
	_module = std::make_shared<x::module>();

	x::code_generater generater;
	for ( auto & it : _objects )
	{
		auto obj = std::static_pointer_cast<x::compiler::object>( it );
		if ( obj->reset )
		{
			obj->reset = false;

			generater.generate( _module, logger(), symbols(), obj->ast );

			logger()->info( std::format( "generate module: {}", obj->path.string() ) );
		}
	}
}

void x::compiler::linkmodule()
{
	for ( auto & it : _modules )
	{
		_module->merge( it );

		logger()->info( std::format( "link module: {}", it->name ) );
	}

	_module->name = _name;
	_module->author = _author;
	_module->origin = _origin;
	_module->version = _version;
}

void x::compiler::compilemodule()
{
	logger()->info( std::format( "compile module: {} success!", _name ) );
}

x::symbols_ptr x::compiler::make_symbols() const
{
	auto symbols = std::make_shared<x::symbols>();

	//////////////////////////////////// foundation ////////////////////////////////////
#define BEG( TYPE ) auto TYPE##_type = symbols->add_foundation( #TYPE, sizeof( TYPE ) ); symbols->push_scope( TYPE##_type );
#define END() symbols->pop_scope();

	using buffer = x::buffer;
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

std::filesystem::path x::compiler::make_stdpath() const
{
	return std::filesystem::path( x_path_app_path() ) / "std";
}

void x::compiler::load_module( const std::string & modulename )
{
	auto it = std::find_if( _modules.begin(), _modules.end(), [&modulename]( const auto & val )
	{
		return val->name == modulename || val->origin == modulename;
	} );
	if ( it != _modules.end() )
		return;

	x::module_ptr module = nullptr;

	if ( modulename == "std" )
		module = load_std_module( modulename );
	else if ( modulename.find( "std." ) == 0 )
		module = load_std_module( modulename );
	else if ( modulename.find( "http://" ) == 0 )
		module = load_network_module( modulename );
	else if ( modulename.find( "https://" ) == 0 )
		module = load_network_module( modulename );
	else
		module = load_filesystem_module( modulename + x::module_extension );

	XTHROW( x::compile_exception, module == nullptr, "" );

	_modules.emplace_back( module );

	symbols()->add_module( module.get() );

	logger()->info( std::format( "load module: {}", module->name ) );
}

x::module_ptr x::compiler::load_std_module( const std::string & modulename )
{
	auto stdpath = make_stdpath();
	std::string modulepath = modulename;
#ifdef _WIN32
	for ( auto i = modulepath.find( '.' ); i != std::string::npos; i = modulepath.find( '.' ) )
	{
		modulepath.replace( i, 1, "\\" );
	}
#endif // _WIN32

	x::compiler compiler( logger() );

	if ( compiler.compile( stdpath / modulepath ) )
	{
		return compiler.module();
	}

	return nullptr;
}

x::module_ptr x::compiler::load_network_module( const std::string & modulename )
{
	x::url uri( modulename );



	return nullptr;
}

x::module_ptr x::compiler::load_filesystem_module( const std::string & modulename )
{
	std::string modulepath = modulename;
#ifdef _WIN32
	for ( auto i = modulepath.find( '/' ); i != std::string::npos; i = modulepath.find( '/' ) )
	{
		modulepath.replace( i, 1, "\\" );
	}
#endif // _WIN32

	std::filesystem::path path = modulepath;

	if ( !std::filesystem::exists( path ) )
	{
		for ( size_t i = 0; i < _linkpaths.size(); ++i )
		{
			path = std::filesystem::path( _linkpaths[i] ) / modulepath;
			if ( std::filesystem::exists( path ) )
				break;

			path = path / x::module_extension;
			if ( std::filesystem::exists( path ) )
				break;
		}
	}

	if ( std::filesystem::exists( path ) )
	{
		auto module = std::make_shared<x::module>();

		std::ifstream ifs( path, std::ios::in | std::ios::binary );
		if ( ifs.is_open() )
		{
			module->load( ifs );
			return module;
		}
	}

	return nullptr;
}

x::llvm_compiler::llvm_compiler( const x::logger_ptr & logger )
	: compiler( logger )
{
}

x::llvm_compiler::~llvm_compiler()
{
}

llvm::module_ptr x::llvm_compiler::llvm_module() const
{
	return _module;
}

void x::llvm_compiler::compilemodule()
{
	_module = std::make_shared<llvm::module>();

	logger()->info( std::format( "compile llvm module: {}", module()->name ) );
}

x::symbols_ptr x::llvm_compiler::make_symbols() const
{
	auto sym = std::make_shared<x::symbols>();

	return sym;
}

std::filesystem::path x::llvm_compiler::make_stdpath() const
{
	return std::filesystem::path( x_path_app_path() ) / "llvm";
}

x::spirv_compiler::spirv_compiler( const x::logger_ptr & logger )
	: compiler( logger )
{
}

x::spirv_compiler::~spirv_compiler()
{
}

spirv::module_ptr x::spirv_compiler::spirv_module() const
{
	return _module;
}

void x::spirv_compiler::compilemodule()
{
	_module = std::make_shared<spirv::module>();

	logger()->info( std::format( "compile spirv module: {}", module()->name ) );
}

x::symbols_ptr x::spirv_compiler::make_symbols() const
{
	auto sym = std::make_shared<x::symbols>();

	return sym;
}

std::filesystem::path x::spirv_compiler::make_stdpath() const
{
	return std::filesystem::path( x_path_app_path() ).parent_path() / "spirv";
}
