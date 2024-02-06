#include "context.h"

#include "pass.h"
#include "grammar.h"
#include "symbols.h"
#include "xstdlib.h"

struct x::context::private_p
{
	int _version = x::version_num;
	std::string _strpool;
	x::symbols_ptr _symbols;
	std::map<uint64_t, meta_ptr> _metas;

	std::vector<std::filesystem::path> _paths;
};

x::context::context()
	: _p( new private_p )
{
}

x::context::~context()
{
	if ( _p ) delete _p;
}

int x::context::version() const
{
	return _p->_version;
}

const x::symbols_ptr & x::context::symbols() const
{
	return _p->_symbols;
}

x::meta_ptr x::context::find_meta( uint64_t hashcode ) const
{
	auto it = _p->_metas.find( hashcode );
	return it == _p->_metas.end() ? nullptr : it->second;
}

void x::context::load_stdlib()
{
	x::load_stdlib( shared_from_this() );
}

bool x::context::load_file( const std::filesystem::path & file )
{
	bool result = false;

	if ( std::filesystem::exists( file ) )
	{
		std::ifstream stream( file );
		if ( stream.is_open() )
		{
			_p->_paths.push_back( file.parent_path() );
			result = load_stream( stream, file.string() );
			_p->_paths.pop_back();
		}
	}

	return result;
}

bool x::context::load_stream( std::istream & stream, std::string_view name )
{
	if ( _p->_symbols == nullptr )
		_p->_symbols = std::make_shared<x::symbols>();

	std::vector<x::unit_ast_ptr> units;

	if ( recursion_import( stream, name, units ) )
	{
		scope_scanner_pass pass1;
		reference_resolver_pass pass2;
		type_checker_pass pass3;
		semantic_validator_pass pass4;

		pass1.set_ctx( this );
		pass2.set_ctx( this );
		pass3.set_ctx( this );
		pass4.set_ctx( this );

		for ( auto it = units.rbegin(); it != units.rend(); ++it ) pass1.visit( it->get() );
		for ( auto it = units.rbegin(); it != units.rend(); ++it ) pass2.visit( it->get() );
		for ( auto it = units.rbegin(); it != units.rend(); ++it ) pass3.visit( it->get() );
		for ( auto it = units.rbegin(); it != units.rend(); ++it ) pass4.visit( it->get() );

		return true;
	}

	return false;
}

void x::context::add_search_path( const std::filesystem::path & path )
{
	_p->_paths.push_back( path );
}

std::filesystem::path x::context::search_path( const std::filesystem::path & path ) const
{
	if ( std::filesystem::exists( path ) )
		return path;

	for ( const auto & it : path )
	{
		auto p = it / path;
		if ( std::filesystem::exists( p ) )
		{
			return p;
		}
	}

	return {};
}

void x::context::register_meta( const meta_ptr & val )
{
	static std::hash<std::string_view> h;
	val->_hashcode = h( { val->fullname().data(), val->fullname().size() } );
	_p->_metas.insert( { val->_hashcode, val } );
}

x::static_string_view x::context::trans_string_view( std::string_view str )
{
	auto it = _p->_strpool.find( str );
	if ( it == std::string::npos )
	{
		it = _p->_strpool.size();
		_p->_strpool.append( str.data(), str.size() );
	}

	return { &_p->_strpool, it, str.size() };
}

bool x::context::recursion_import( std::istream & stream, std::filesystem::path path, std::vector<x::unit_ast_ptr> & units )
{
	bool result = false;

	if ( auto unit = x::grammar( stream, path.string() ).unit() )
	{
		for ( const auto & it : unit->imports )
		{
			if ( std::find_if( units.begin(), units.end(), [&path]( const auto & val ) { return val->location.file == path; } ) == units.end() )
			{
				auto p = search_path( it->path );

				if ( !p.empty() )
				{
					std::ifstream fs( p );
					if ( fs.is_open() )
					{
						_p->_paths.push_back( p.parent_path() );
						result = recursion_import( stream, p, units );
						_p->_paths.pop_back();

						units.emplace_back( unit );
					}
				}
			}
		}
	}

	return true;
}
