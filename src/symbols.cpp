#include "symbols.h"

#include <regex>

#define REG_BASIC( NAME ) \
{ \
	auto sym = new x::symbol; \
	sym->ast = nullptr; \
	sym->name = #NAME; \
	sym->parent = _global; \
	sym->type = x::symbol_t::CLASS; \
	_global->symbols.push_back( sym ); \
}

x::symbols::symbols()
	: _scopes( 1 )
{
	_global = new x::scope;

	_global->type = x::symbol_t::NAMESPACE;
	_global->name = "";
	_global->parent = nullptr;

	_cur = _global;

	REG_BASIC( void );
	REG_BASIC( byte );
	REG_BASIC( bool );
	REG_BASIC( any );
	REG_BASIC( int );
	REG_BASIC( float );
	REG_BASIC( string );
}

x::symbols::~symbols()
{
}

void x::symbols::beg_unit()
{
	_cur = _global;
}

void x::symbols::end_unit()
{
}

void x::symbols::push_scope( x::symbol_t type, std::string_view name, x::ast * ast )
{
	x::scope * sym = nullptr;

	auto it = std::find_if( _cur->symbols.begin(), _cur->symbols.end(), [name](const x::symbol * val ) { return val->name == name; } );

	if ( it == _cur->symbols.end() )
	{
		sym = new x::scope;
		sym->ast = ast;
		sym->type = type;
		sym->name = name;
		sym->parent = _cur;
		_cur->symbols.push_back( sym );
	}
	else
	{
		ASSERT( ( *it )->type != type, "" );

		sym = (x::scope *)*it;
	}

	_cur = sym;
}

void x::symbols::add_symbol( x::symbol_t type, std::string_view name, x::ast * ast )
{
	ASSERT( std::find_if( _cur->symbols.begin(), _cur->symbols.end(), [name]( const x::symbol * val ) { return val->name == name; } ) != _cur->symbols.end(), "" );

	auto sym = new x::symbol;
	sym->ast = ast;
	sym->type = type;
	sym->name = name;
	sym->parent = _cur;
	_cur->symbols.emplace_back( sym );
}

void x::symbols::pop_scope()
{
	_cur = (x::scope *)_cur->parent;
}

x::scope * x::symbols::global_scope() const
{
	return _global;
}

bool x::symbols::is_scope( x::symbol * sym ) const
{
	if ( sym != nullptr )
	{
		switch ( sym->type )
		{
		case x::symbol_t::ENUM:
		case x::symbol_t::LOOP:
		case x::symbol_t::BLOCK:
		case x::symbol_t::CLASS:
		case x::symbol_t::FUNCTION:
		case x::symbol_t::NAMESPACE:
			return true;
		}
	}

	return false;
}

x::scope * x::symbols::find_scope( x::symbol_t type ) const
{
	auto result = _cur;

	while ( result != nullptr && result->type != type )
		result = (x::scope *)result->parent;

	return result;
}

x::symbol * x::symbols::find_symbol( std::string_view name, x::scope * scope ) const
{
	x::symbol * sym = ( scope == nullptr ) ? _cur : scope;

	std::regex reg( "." );
	std::cregex_token_iterator it( name.data(), name.data() + name.size(), reg, -1 ), end;

	for ( ; it != end; ++it )
	{
		ASSERT( !is_scope( sym ), "" );

		sym = find_symbol( (x::scope *)sym, { it->first, it->second } );
	}

	return sym;
}

x::symbol * x::symbols::find_reference( x::ast * ast ) const
{
	auto it = _references.find( ast );

	return it != _references.end() ? it->second : nullptr;
}

void x::symbols::add_reference( x::ast * ast, x::symbol * sym )
{
	_references[ast] = sym;
}

x::symbol * x::symbols::find_symbol( x::scope * scope, std::string_view name ) const
{
	if ( scope == nullptr )
		return nullptr;

	auto it = std::find_if( scope->symbols.begin(), scope->symbols.end(), [name]( const x::symbol * val )
	{
		return val->name == name;
	} );
	if ( it != scope->symbols.end() )
		return *it;

	return find_symbol( (x::scope *)scope->parent, name );
}
