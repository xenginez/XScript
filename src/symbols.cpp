#include "symbols.h"

#include <regex>

#include "ast.h"
#include "value.h"

namespace
{
	template <class _Container> auto name_beg( const _Container & _Cont ) -> decltype( std::begin( _Cont ) )
	{
		return  std::begin( _Cont );
	}

	template <class _Container, class _Iterator = _Container::const_iterator> _Iterator name_end( _Container & _Cont, _Iterator _Off = {} )
	{
		size_t off = 0;
		if ( _Off != _Iterator{} )
			off = std::distance( std::begin( _Cont ), _Off );

		auto i = _Cont.find( '.', off );
		if ( i != _Container::npos )
			return begin( _Cont ) + i;
		return std::end( _Cont );
	}

	class builtin_symbol
	{

	};
	class foundation_symbol : public x::class_symbol
	{
	public:
		foundation_symbol( std::string_view name, x::uint64 size )
			:_size( size )
		{
			this->name = name;
			this->fullname = name;
		}

	public:
		x::uint64 size() const override
		{
			return _size;
		}
		
	private:
		x::uint64 _size = 0;
	};
}

bool x::symbol::is_type() const
{
	switch ( type )
	{
	case x::symbol_t::ENUM:
	case x::symbol_t::ALIAS:
	case x::symbol_t::CLASS:
	case x::symbol_t::TEMPLATE:
		return true;
	}
	return false;
}

bool x::symbol::is_scope() const
{
	switch ( type )
	{
	case x::symbol_t::UNIT:
	case x::symbol_t::ENUM:
	case x::symbol_t::CLASS:
	case x::symbol_t::BLOCK:
	case x::symbol_t::CYCLE:
	case x::symbol_t::FUNCTION:
	case x::symbol_t::TEMPLATE:
	case x::symbol_t::NAMESPACE:
		return true;
	}
	return false;
}

bool x::symbol::is_variable() const
{
	switch ( type )
	{
	case x::symbol_t::LOCAL:
	case x::symbol_t::PARAM:
	case x::symbol_t::VARIABLE:
		return true;
	}
	return false;
}

bool x::symbol::is_function() const
{
	return type == x::symbol_t::FUNCTION;
}

x::type_symbol * x::symbol::cast_type()
{
	ASSERT( is_type(), "" );

	return reinterpret_cast<x::type_symbol *>( this );
}

x::scope_symbol * x::symbol::cast_scope()
{
	ASSERT( is_scope(), "" );

	return reinterpret_cast<x::scope_symbol *>( this );
}

x::local_symbol * x::symbol::cast_local()
{
	return dynamic_cast<x::local_symbol *>( this );
}

x::param_symbol * x::symbol::cast_param()
{
	return dynamic_cast<x::param_symbol *>( this );
}

x::variable_symbol * x::symbol::cast_variable()
{
	return dynamic_cast<x::variable_symbol *>( this );
}

x::function_symbol * x::symbol::cast_function()
{
	return dynamic_cast<x::function_symbol *>( this );
}

x::symbol * x::type_symbol::cast_symbol()
{
	return reinterpret_cast<x::symbol *>( this );
}

x::symbol * x::scope_symbol::cast_symbol()
{
	return reinterpret_cast<x::symbol *>( this );
}

x::unit_symbol::unit_symbol()
{
	type = x::symbol_t::UNIT;
}

x::unit_symbol::~unit_symbol()
{
}

x::unit_ast_ptr x::unit_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::unit_ast>( ast );
}

void x::unit_symbol::add_child( x::symbol * val )
{
	children.push_back( val );
}

x::symbol * x::unit_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val ) { return val->name == name; } );
	return ( it != children.end() ) ? *it : nullptr;
}

x::enum_symbol::enum_symbol()
{
	type = x::symbol_t::ENUM;
}

x::enum_symbol::~enum_symbol()
{
}

x::enum_decl_ast_ptr x::enum_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::enum_decl_ast>( ast );
}

x::uint64 x::enum_symbol::size() const
{
	return x::uint64();
}

void x::enum_symbol::add_child( x::symbol * val )
{
	ASSERT( val->type == x::symbol_t::ELEMENT, "" );

	elements.push_back( static_cast<x::element_symbol *>( val ) );
}

x::symbol * x::enum_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( elements.begin(), elements.end(), [name]( auto val ) { return val->name == name; } );
	return ( it != elements.end() ) ? *it : nullptr;
}

x::alias_symbol::alias_symbol()
{
	type = x::symbol_t::ALIAS;
}

x::alias_symbol::~alias_symbol()
{
}

x::using_decl_ast_ptr x::alias_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::using_decl_ast>( ast );
}

x::uint64 x::alias_symbol::size() const
{
	return retype->size();
}

x::class_symbol::class_symbol()
{
	type = x::symbol_t::CLASS;
}

x::class_symbol::~class_symbol()
{
}

x::class_decl_ast_ptr x::class_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::class_decl_ast>( ast );
}

x::uint64 x::class_symbol::size() const
{
	return 0;

	/*
	x::uint64 sz = 1;

	if ( base != nullptr )
		sz = base->size();

	for ( auto it : variables )
	{
		auto ast = it->cast_ast();

		if ( ast->is_static || ast->is_thread )
			continue;

		auto val_type_sz = it->value->size();

		if ( ast->is_local )
			sz += val_type_sz * ast->value_type->desc.array;
		else
			sz += reference_size;
	}
	return ALIGN( sz, reference_size );
	*/
}

void x::class_symbol::add_child( x::symbol * val )
{
	switch ( val->type )
	{
	case x::symbol_t::ALIAS:
		aliases.push_back( static_cast<x::alias_symbol *>( val ) );
		break;
	case x::symbol_t::FUNCTION:
		functions.push_back( static_cast<x::function_symbol *>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( static_cast<x::variable_symbol *>( val ) );
		break;
	default:
		ASSERT( false, "" );
		break;
	}
}

x::symbol * x::class_symbol::find_child( std::string_view name ) const
{
	auto ait = std::find_if( aliases.begin(), aliases.end(), [name]( auto val ) { return val->name == name; } );
	if ( ait != aliases.end() )
		return *ait;

	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val ) { return val->name == name; } );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val ) { return val->name == name; } );
	if ( fit != functions.end() )
		return *fit;

	return nullptr;
}

x::block_symbol::block_symbol()
{
	type = x::symbol_t::BLOCK;
}

x::block_symbol::~block_symbol()
{
}

x::compound_stat_ast_ptr x::block_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::compound_stat_ast>( ast );
}

void x::block_symbol::add_child( x::symbol * val )
{
	switch ( val->type )
	{
	case x::symbol_t::LOCAL:
		locals.push_back( static_cast<x::local_symbol *>( val ) );
		break;
	case x::symbol_t::BLOCK:
	case x::symbol_t::CYCLE:
		blocks.push_back( static_cast<x::block_symbol *>( val ) );
		break;
	}
}

x::symbol * x::block_symbol::find_child( std::string_view name ) const
{
	auto bit = std::find_if( blocks.begin(), blocks.end(), [name]( auto val ) { return val->name == name; } );
	if ( bit != blocks.end() )
		return *bit;
	
	auto lit = std::find_if( locals.begin(), locals.end(), [name]( auto val ) { return val->name == name; } );
	if ( lit != locals.end() )
		return *lit;

	return nullptr;
}

x::cycle_symbol::cycle_symbol()
{
	type = x::symbol_t::CYCLE;
}

x::cycle_symbol::~cycle_symbol()
{
}

x::cycle_stat_ast_ptr x::cycle_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::cycle_stat_ast>( ast );
}

x::local_symbol::local_symbol()
{
	type = x::symbol_t::LOCAL;
}

x::local_symbol::~local_symbol()
{
}

x::local_stat_ast_ptr x::local_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::local_stat_ast>( ast );
}

x::param_symbol::param_symbol()
{
	type = x::symbol_t::PARAM;
}

x::param_symbol::~param_symbol()
{
}

x::parameter_decl_ast_ptr x::param_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::parameter_decl_ast>( ast );
}

x::function_symbol::function_symbol()
{
	type = x::symbol_t::FUNCTION;
}

x::function_symbol::~function_symbol()
{
}

x::function_decl_ast_ptr x::function_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::function_decl_ast>( ast );
}

void x::function_symbol::add_child( x::symbol * val )
{
	ASSERT( val->type == x::symbol_t::PARAM, "" );

	parameters.push_back( static_cast<x::param_symbol *>( val ) );
}

x::symbol * x::function_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val ) { return val->name == name; } );
	return ( it != parameters.end() ) ? *it : nullptr;
}

x::variable_symbol::variable_symbol()
{
	type = x::symbol_t::VARIABLE;
}

x::variable_symbol::~variable_symbol()
{
}

x::variable_decl_ast_ptr x::variable_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::variable_decl_ast>( ast );
}

x::template_symbol::template_symbol()
{
	type = x::symbol_t::TEMPLATE;
}

x::template_symbol::~template_symbol()
{
}

x::template_decl_ast_ptr x::template_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::template_decl_ast>( ast );
}

x::uint64 x::template_symbol::size() const
{
	return x::uint64();
}

void x::template_symbol::add_child( x::symbol * val )
{
	switch ( val->type )
	{
	case x::symbol_t::ALIAS:
		aliases.push_back( static_cast<x::alias_symbol *>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( static_cast<x::variable_symbol *>( val ) );
		break;
	case x::symbol_t::FUNCTION:
		functions.push_back( static_cast<x::function_symbol *>( val ) );
		break;
	default:
		ASSERT( false, "" );
		break;
	}
}

x::symbol * x::template_symbol::find_child( std::string_view name ) const
{
	auto ait = std::find_if( aliases.begin(), aliases.end(), [name]( auto val ) { return val->name == name; } );
	if ( ait != aliases.end() )
		return *ait;

	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val ) { return val->name == name; } );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val ) { return val->name == name; } );
	if ( fit != functions.end() )
		return *fit;
	
	return nullptr;
}

x::namespace_symbol::namespace_symbol()
{
	type = x::symbol_t::NAMESPACE;
}

x::namespace_symbol::~namespace_symbol()
{
}

x::namespace_decl_ast_ptr x::namespace_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::namespace_decl_ast>( ast );
}

void x::namespace_symbol::add_child( x::symbol * val )
{
	ASSERT( val->is_type(), "" );

	children.push_back( val->cast_type() );
}

x::symbol * x::namespace_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val ) { return val->cast_symbol()->name == name; } );
	return ( it != children.end() ) ? (*it)->cast_symbol() : nullptr;
}

x::element_symbol::element_symbol()
{
	type = x::symbol_t::ELEMENT;
}

x::element_symbol::~element_symbol()
{
}

x::element_decl_ast_ptr x::element_symbol::cast_ast() const
{
	return std::static_pointer_cast<x::element_decl_ast>( ast );
}

x::symbols::symbols()
{
	auto val = new namespace_symbol;
	_symbolmap[""] = val;
	_scope.push_back( val );
	{
		x::symbol * sym = nullptr;

#define FOUNDATION( TYPE ) sym = new foundation_symbol( #TYPE, sizeof( TYPE ) ); add_symbol( sym );
		sym = new foundation_symbol( "any", sizeof( x::value ) ); add_symbol( sym );
		sym = new foundation_symbol( "void", 0 ); add_symbol( sym );
		FOUNDATION( bool );
		FOUNDATION( byte );
		FOUNDATION( int8 );
		FOUNDATION( int16 );
		FOUNDATION( int32 );
		FOUNDATION( int64 );
		FOUNDATION( uint8 );
		FOUNDATION( uint16 );
		FOUNDATION( uint32 );
		FOUNDATION( uint64 );
		FOUNDATION( float16 );
		FOUNDATION( float32 );
		FOUNDATION( float64 );
		FOUNDATION( string );
		FOUNDATION( intptr );
		sym = new foundation_symbol( "object", sizeof( x::object * ) ); add_symbol( sym );
#undef FOUNDATION
	}
}

x::symbols::~symbols()
{
	for ( auto it : _symbolmap )
		delete it.second;
}

void x::symbols::push_scope( std::string_view name )
{
	auto sym = current_scope()->find_child( name );
	ASSERT( sym && sym->is_scope(), "" );
	_scope.push_back( sym->cast_scope() );
}

x::unit_symbol * x::symbols::add_unit( x::unit_ast * ast )
{
	std::string fullname = { ast->location.file.data(), ast->location.file.size() };

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new unit_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = fullname;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::enum_symbol * x::symbols::add_enum( x::enum_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new enum_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::alias_symbol * x::symbols::add_alias( x::using_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new alias_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::class_symbol * x::symbols::add_class( x::class_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new class_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::block_symbol * x::symbols::add_block( x::compound_stat_ast * ast )
{
	std::string fullname = std::format( "block_{}_{}_{}", ast->location.file, ast->location.line, ast->location.column );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new block_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = fullname;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::cycle_symbol * x::symbols::add_cycle( x::cycle_stat_ast * ast )
{
	std::string fullname = std::format( "cycle_{}_{}_{}", ast->location.file, ast->location.line, ast->location.column );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new cycle_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = fullname;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::local_symbol * x::symbols::add_local( x::local_stat_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new local_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::param_symbol * x::symbols::add_param( x::parameter_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new param_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::element_symbol * x::symbols::add_element( x::element_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new element_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::function_symbol * x::symbols::add_function( x::function_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new function_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::variable_symbol * x::symbols::add_variable( x::variable_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new variable_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::template_symbol * x::symbols::add_template( x::template_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new template_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::namespace_symbol * x::symbols::add_namespace( x::namespace_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new namespace_symbol;

	sym->ast = ast->shared_from_this();
	sym->name = ast->name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::type_symbol * x::symbols::find_type_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	ASSERT( sym != nullptr && sym->is_type(), "" );
	return sym->cast_type();
}

x::scope_symbol * x::symbols::find_scope_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	ASSERT( sym != nullptr && sym->is_scope(), "" );
	return sym->cast_scope();
}

x::class_symbol * x::symbols::find_class_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	ASSERT( sym != nullptr && sym->type == x::symbol_t::CLASS, "" );
	return reinterpret_cast<x::class_symbol *>( sym );
}

std::vector<x::template_symbol *> x::symbols::find_template_symbols( std::string_view name ) const
{
	std::vector<x::template_symbol *> result;

	auto pair = _templatemap.equal_range( calc_fullname( name ) );
	for ( auto it = pair.first; it != pair.second; ++it )
	{
		result.push_back( it->second );
	}
	
	return result;
}

x::symbol * x::symbols::find_symbol( std::string_view name, x::scope_symbol * scope ) const
{
	if ( scope == nullptr )
		scope = current_scope();

	if ( name.empty() )
		return scope->cast_symbol();

	if ( scope->cast_symbol()->name == name )
		return scope->cast_symbol();

	if ( auto sym = down_find_symbol( name, scope ) )
		return sym;
	else if ( auto sym = up_find_symbol( name, scope ) )
		return sym;

	return nullptr;
}

x::symbol * x::symbols::up_find_symbol_from_type( x::symbol_t type ) const
{
	auto scope = current_scope();

	while ( scope )
	{
		if ( scope->cast_symbol()->type == type )
			return scope->cast_symbol();

		auto parent = scope->cast_symbol()->parent;
		if ( parent && parent->is_scope() )
			scope = parent->cast_scope();
		else
			scope = nullptr;
	}

	return nullptr;
}

x::symbol * x::symbols::up_find_symbol( std::string_view name, x::scope_symbol * scope ) const
{
	auto beg = name_beg( name );
	auto end = name_end( name );
	std::string_view nname{ beg, end };

	if ( auto parent = scope->cast_symbol()->parent )
	{
		if ( parent->name == nname )
		{
			if ( end == name.end() )
				return parent;
			else
				return down_find_symbol( { end + 1, name_end( name, end + 1 ) }, parent->cast_scope() );
		}
		else
		{
			return up_find_symbol( name, parent->cast_scope() );
		}
	}

	return nullptr;
}

x::symbol * x::symbols::down_find_symbol( std::string_view name, x::scope_symbol * scope ) const
{
	auto beg = name_beg( name );
	auto end = name_end( name );
	std::string_view nname{ beg, end };

	if ( auto child = scope->find_child( nname ) )
	{
		if ( end == name.end() )
			return child;
		else if ( child->is_scope() )
			return down_find_symbol( { end + 1, name_end( name, end + 1 ) }, child->cast_scope() );
	}

	return nullptr;
}

x::scope_symbol * x::symbols::current_scope() const
{
	return _scope.back();
}

void x::symbols::pop_scope()
{
	_scope.pop_back();
}

x::namespace_symbol * x::symbols::global_namespace() const
{
	return reinterpret_cast<x::namespace_symbol *>( _symbolmap.at( "" ) );
}

std::string x::symbols::calc_fullname( std::string_view name ) const
{
	std::string fullname;

	if ( auto sym = current_scope()->cast_symbol() )
	{
		if ( sym->type == x::symbol_t::UNIT || ( sym->type == x::symbol_t::NAMESPACE && sym->name.empty() ) )
			fullname = name;
		else
			fullname = std::format( "{}.{}", sym->fullname, name );
	}

	return fullname;
}

x::symbol * x::symbols::find_symbol_from_ast( const x::ast_ptr & ast ) const
{
	auto it = _astmap.find( ast );
	return it != _astmap.end() ? it->second : nullptr;
}

x::symbol * x::symbols::find_symbol_from_fullname( std::string_view fullname ) const
{
	auto it = _symbolmap.find( { fullname.begin(), fullname.end() } );

	return it != _symbolmap.end() ? it->second : nullptr;
}

void x::symbols::add_reference( std::string_view name, x::symbol * val )
{
	_referencemap.emplace( name, val );
}

x::symbol * x::symbols::find_reference( std::string_view name ) const
{
	auto it = _referencemap.find( { name.begin(), name.end() } );
	
	return it != _referencemap.end() ? it->second : nullptr;
}

void x::symbols::add_symbol( x::symbol * val )
{
	val->parent = current_scope()->cast_symbol();

	_astmap[val->ast] = val;
	_symbolmap[val->fullname] = val;

	if ( val->type == x::symbol_t::TEMPLATE )
		_templatemap.emplace( val->fullname, static_cast<x::template_symbol *>( val ) );

	current_scope()->add_child( val );
}
