#include "symbols.h"

#include <regex>

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
}

bool x::symbol::is_type() const
{
	switch ( type )
	{
	case x::symbol_t::ENUM:
	case x::symbol_t::FLAG:
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
	case x::symbol_t::FLAG:
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

x::uint64 x::enum_symbol::size() const
{
	return x::uint64();
}

void x::enum_symbol::add_child( x::symbol * val )
{
	ASSERT( val->type == x::symbol_t::ENUM_ELEMENT, "" );

	elements.push_back( static_cast<x::enum_element_symbol *>( val ) );
}

x::symbol * x::enum_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( elements.begin(), elements.end(), [name]( auto val ) { return val->name == name; } );
	return ( it != elements.end() ) ? *it : nullptr;
}

x::flag_symbol::flag_symbol()
{
	type = x::symbol_t::FLAG;
}

x::flag_symbol::~flag_symbol()
{
}

x::uint64 x::flag_symbol::size() const
{
	return x::uint64();
}

void x::flag_symbol::add_child( x::symbol * val )
{
	ASSERT( val->type == x::symbol_t::FLAG_ELEMENT, "" );

	elements.push_back( static_cast<x::flag_element_symbol *>( val ) );
}

x::symbol * x::flag_symbol::find_child( std::string_view name ) const
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

x::uint64 x::alias_symbol::size() const
{
	return value->size();
}

x::class_symbol::class_symbol()
{
	type = x::symbol_t::CLASS;
}

x::class_symbol::~class_symbol()
{
}

x::uint64 x::class_symbol::size() const
{
	x::uint64 sz = 1;

	if ( base != nullptr )
		sz = base->size();

	for ( auto it : variables )
	{
		if ( it->is_static || it->is_thread )
			continue;

		sz = it->value->size();
	}
	return ALIGN( sz, 8 );
}

void x::class_symbol::add_child( x::symbol * val )
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

void x::block_symbol::add_child( x::symbol * val )
{
	children.push_back( val );
}

x::symbol * x::block_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val ) { return val->name == name; } );
	return ( it != children.end() ) ? *it : nullptr;
}

x::cycle_symbol::cycle_symbol()
{
	type = x::symbol_t::CYCLE;
}

x::cycle_symbol::~cycle_symbol()
{
}

x::local_symbol::local_symbol()
{
	type = x::symbol_t::LOCAL;
}

x::local_symbol::~local_symbol()
{
}

x::param_symbol::param_symbol()
{
	type = x::symbol_t::PARAM;
}

x::param_symbol::~param_symbol()
{
}

x::function_symbol::function_symbol()
{
	type = x::symbol_t::FUNCTION;
}

x::function_symbol::~function_symbol()
{
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

x::template_symbol::template_symbol()
{
	type = x::symbol_t::TEMPLATE;
}

x::template_symbol::~template_symbol()
{
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
	case x::symbol_t::TEMP_ELEMENT:
		elements.push_back( static_cast<x::temp_element_symbol *>( val ) );
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
	
	auto eit = std::find_if( elements.begin(), elements.end(), [name]( auto val ) { return val->name == name; } );
	if ( eit != elements.end() )
		return *eit;

	return nullptr;
}

x::namespace_symbol::namespace_symbol()
{
	type = x::symbol_t::NAMESPACE;
}

x::namespace_symbol::~namespace_symbol()
{
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

x::enum_element_symbol::enum_element_symbol()
{
	type = x::symbol_t::ENUM_ELEMENT;
}

x::enum_element_symbol::~enum_element_symbol()
{
}

x::flag_element_symbol::flag_element_symbol()
{
	type = x::symbol_t::FLAG_ELEMENT;
}

x::flag_element_symbol::~flag_element_symbol()
{
}

x::temp_element_symbol::temp_element_symbol()
{
	type = x::symbol_t::TEMP_ELEMENT;
}

x::temp_element_symbol::~temp_element_symbol()
{
}

x::symbols::symbols()
{
	auto val = new namespace_symbol;
	_symbolmap[""] = val;
	_scope.push_back( val );
	{
		auto void_symbol = add_class( "void", {} );
		auto byte_symbol = add_class( "byte", {} );
		auto bool_symbol = add_class( "bool", {} );
		auto any_symbol = add_class( "any", {} );
		auto intptr_symbol = add_class( "intptr", {} );
		auto int8_symbol = add_class( "int8", {} );
		auto int16_symbol = add_class( "int16", {} );
		auto int32_symbol = add_class( "int32", {} );
		auto int64_symbol = add_class( "int64", {} );
		auto uint8_symbol = add_class( "uint8", {} );
		auto uint16_symbol = add_class( "uint16", {} );
		auto uint32_symbol = add_class( "uint32", {} );
		auto uint64_symbol = add_class( "uint64", {} );
		auto float16_symbol = add_class( "float16", {} );
		auto float32_symbol = add_class( "float32", {} );
		auto float64_symbol = add_class( "float64", {} );
		auto string_symbol = add_class( "string", {} );
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

x::unit_symbol * x::symbols::add_unit( const x::location & location )
{
	std::string fullname = { location.file.data(), location.file.size() };

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new unit_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::enum_symbol * x::symbols::add_enum( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new enum_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::flag_symbol * x::symbols::add_flag( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new flag_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::alias_symbol * x::symbols::add_alias( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new alias_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::class_symbol * x::symbols::add_class( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new class_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::block_symbol * x::symbols::add_block( const x::location & location )
{
	std::string fullname = std::format( "block_{}_{}_{}", location.file, location.line, location.column );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new block_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::cycle_symbol * x::symbols::add_cycle( const x::location & location )
{
	std::string fullname = std::format( "cycle_{}_{}_{}", location.file, location.line, location.column );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new cycle_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::local_symbol * x::symbols::add_local( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new local_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::param_symbol * x::symbols::add_param( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new param_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::function_symbol * x::symbols::add_function( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new function_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::variable_symbol * x::symbols::add_variable( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new variable_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::template_symbol * x::symbols::add_template( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new template_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::namespace_symbol * x::symbols::add_namespace( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new namespace_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::enum_element_symbol * x::symbols::add_enum_element( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new enum_element_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::flag_element_symbol * x::symbols::add_flag_element( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new flag_element_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

x::temp_element_symbol * x::symbols::add_temp_element( std::string_view name, const x::location & location )
{
	std::string fullname = calc_fullname( name );

	ASSERT( _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new temp_element_symbol;
	sym->name = fullname;
	sym->fullname = fullname;
	sym->location = location;
	sym->parent = current_scope()->cast_symbol();

	_symbolmap[sym->fullname] = sym;
	current_scope()->add_child( sym );

	return sym;
}

bool x::symbols::has_symbol( std::string_view name ) const
{
	return find_symbol( name ) != nullptr;
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

x::symbol * x::symbols::find_symbol_from_fullname( std::string_view fullname ) const
{
	auto it = _symbolmap.find( { fullname.begin(), fullname.end() } );

	return it != _symbolmap.end() ? it->second : nullptr;
}

void x::symbols::add_reference( const x::location & location, x::symbol * val )
{
}

x::symbol * x::symbols::find_reference( const x::location & location ) const
{
	return nullptr;
}
