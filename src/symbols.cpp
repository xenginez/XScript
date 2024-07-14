#include "symbols.h"

#include <regex>

#include "ast.h"
#include "value.h"
#include "builtin.h"
#include "exception.h"

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

bool x::symbol::is_ast() const
{
	switch ( type )
	{
	case x::symbol_t::BUILTINFUNC:
	case x::symbol_t::FOUNDATION:
		return false;
	}
	return true;
}

bool x::symbol::is_type() const
{
	switch ( type )
	{
	case x::symbol_t::ENUM:
	case x::symbol_t::ALIAS:
	case x::symbol_t::CLASS:
	case x::symbol_t::FOUNDATION:
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
	case x::symbol_t::FOUNDATION:
		return true;
	}
	return false;
}

bool x::symbol::is_unit() const
{
	return type == x::symbol_t::UNIT;
}

bool x::symbol::is_enum() const
{
	return type == x::symbol_t::ENUM;
}

bool x::symbol::is_alias() const
{
	return type == x::symbol_t::ALIAS;
}

bool x::symbol::is_class() const
{
	return type == x::symbol_t::CLASS;
}

bool x::symbol::is_block() const
{
	return type == x::symbol_t::BLOCK;
}

bool x::symbol::is_cycle() const
{
	return type == x::symbol_t::CYCLE;
}

bool x::symbol::is_local() const
{
	return type == x::symbol_t::LOCAL;
}

bool x::symbol::is_param() const
{
	return type == x::symbol_t::PARAM;
}

bool x::symbol::is_element() const
{
	return type == x::symbol_t::ELEMENT;
}

bool x::symbol::is_function() const
{
	switch ( type )
	{
	case x::symbol_t::FUNCTION:
	case x::symbol_t::NATIVEFUNC:
	case x::symbol_t::BUILTINFUNC:
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

bool x::symbol::is_template() const
{
	return type == x::symbol_t::TEMPLATE;
}

bool x::symbol::is_namespace() const
{
	return type == x::symbol_t::NAMESPACE;
}

bool x::symbol::is_foundation() const
{
	return type == x::symbol_t::FOUNDATION;
}

bool x::symbol::is_nativefunc() const
{
	return type == x::symbol_t::NATIVEFUNC;
}

bool x::symbol::is_builtinfunc() const
{
	return type == x::symbol_t::BUILTINFUNC;
}

x::ast_symbol * x::symbol::cast_ast()
{
	XTHROW( x::semantic_exception, is_ast(), "" );

	return reinterpret_cast<x::ast_symbol *>( this );
}

x::type_symbol * x::symbol::cast_type()
{
	XTHROW( x::semantic_exception, is_type(), "" );

	return reinterpret_cast<x::type_symbol *>( this );
}

x::scope_symbol * x::symbol::cast_scope()
{
	XTHROW( x::semantic_exception, is_scope(), "" );

	return reinterpret_cast<x::scope_symbol *>( this );
}

x::unit_symbol * x::symbol::cast_unit()
{
	XTHROW( x::semantic_exception, is_unit(), "" );

	return dynamic_cast<x::unit_symbol *>( this );
}

x::enum_symbol * x::symbol::cast_enum()
{
	XTHROW( x::semantic_exception, is_enum(), "" );

	return dynamic_cast<x::enum_symbol *>( this );
}

x::alias_symbol * x::symbol::cast_alias()
{
	XTHROW( x::semantic_exception, is_alias(), "" );

	return dynamic_cast<x::alias_symbol *>( this );
}

x::class_symbol * x::symbol::cast_class()
{
	XTHROW( x::semantic_exception, is_class(), "" );

	return dynamic_cast<x::class_symbol *>( this );
}

x::block_symbol * x::symbol::cast_block()
{
	XTHROW( x::semantic_exception, is_block(), "" );

	return dynamic_cast<x::block_symbol *>( this );
}

x::cycle_symbol * x::symbol::cast_cycle()
{
	XTHROW( x::semantic_exception, is_cycle(), "" );

	return dynamic_cast<x::cycle_symbol *>( this );
}

x::local_symbol * x::symbol::cast_local()
{
	XTHROW( x::semantic_exception, is_local(), "" );

	return dynamic_cast<x::local_symbol *>( this );
}

x::param_symbol * x::symbol::cast_param()
{
	XTHROW( x::semantic_exception, is_param(), "" );

	return dynamic_cast<x::param_symbol *>( this );
}

x::element_symbol * x::symbol::cast_element()
{
	XTHROW( x::semantic_exception, is_element(), "" );

	return dynamic_cast<x::element_symbol *>( this );
}

x::function_symbol * x::symbol::cast_function()
{
	XTHROW( x::semantic_exception, is_function(), "" );

	return dynamic_cast<x::function_symbol *>( this );
}

x::variable_symbol * x::symbol::cast_variable()
{
	XTHROW( x::semantic_exception, is_variable(), "" );

	return dynamic_cast<x::variable_symbol *>( this );
}

x::template_symbol * x::symbol::cast_template()
{
	XTHROW( x::semantic_exception, is_template(), "" );

	return dynamic_cast<x::template_symbol *>( this );
}

x::namespace_symbol * x::symbol::cast_namespace()
{
	XTHROW( x::semantic_exception, is_namespace(), "" );

	return dynamic_cast<x::namespace_symbol *>( this );
}

x::foundation_symbol * x::symbol::cast_foundation()
{
	XTHROW( x::semantic_exception, is_foundation(), "" );

	return dynamic_cast<x::foundation_symbol *>( this );
}

x::nativefunc_symbol * x::symbol::cast_nativefunc()
{
	XTHROW( x::semantic_exception, is_nativefunc(), "" );

	return dynamic_cast<x::nativefunc_symbol *>( this );
}

x::builtinfunc_symbol * x::symbol::cast_builtinfunc()
{
	XTHROW( x::semantic_exception, is_builtinfunc(), "" );

	return dynamic_cast<x::builtinfunc_symbol *>( this );
}

x::symbol * x::ast_symbol::cast_symbol()
{
	return reinterpret_cast<x::symbol *>( this );
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

x::ast_ptr x::unit_symbol::ast() const
{
	return unit_ast;
}

x::unit_ast_ptr x::unit_symbol::cast_ast() const
{
	return unit_ast;
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

x::ast_ptr x::enum_symbol::ast() const
{
	return enum_ast;
}

x::enum_decl_ast_ptr x::enum_symbol::cast_ast() const
{
	return enum_ast;
}

x::uint64 x::enum_symbol::size() const
{
	return x::uint64();
}

void x::enum_symbol::add_child( x::symbol * val )
{
	XTHROW( x::semantic_exception, val->type == x::symbol_t::ELEMENT, "" );

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

x::ast_ptr x::alias_symbol::ast() const
{
	return using_ast;
}

x::using_decl_ast_ptr x::alias_symbol::cast_ast() const
{
	return using_ast;
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

x::ast_ptr x::class_symbol::ast() const
{
	return class_ast;
}

x::class_decl_ast_ptr x::class_symbol::cast_ast() const
{
	return class_ast;
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
		XTHROW( x::semantic_exception, false, "" );
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

x::ast_ptr x::block_symbol::ast() const
{
	return compound_ast;
}

x::compound_stat_ast_ptr x::block_symbol::cast_ast() const
{
	return compound_ast;
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

x::ast_ptr x::cycle_symbol::ast() const
{
	return cycle_ast;
}

x::cycle_stat_ast_ptr x::cycle_symbol::cast_ast() const
{
	return cycle_ast;
}

void x::cycle_symbol::add_child( x::symbol * val )
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

x::symbol * x::cycle_symbol::find_child( std::string_view name ) const
{
	auto bit = std::find_if( blocks.begin(), blocks.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( bit != blocks.end() )
		return *bit;

	auto lit = std::find_if( locals.begin(), locals.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( lit != locals.end() )
		return *lit;

	return nullptr;
}

x::local_symbol::local_symbol()
{
	type = x::symbol_t::LOCAL;
}

x::local_symbol::~local_symbol()
{
}

x::ast_ptr x::local_symbol::ast() const
{
	return local_ast;
}

x::local_stat_ast_ptr x::local_symbol::cast_ast() const
{
	return local_ast;
}

x::param_symbol::param_symbol()
{
	type = x::symbol_t::PARAM;
}

x::param_symbol::~param_symbol()
{
}

x::ast_ptr x::param_symbol::ast() const
{
	return parameter_ast;
}

x::parameter_decl_ast_ptr x::param_symbol::cast_ast() const
{
	return parameter_ast;
}

x::element_symbol::element_symbol()
{
	type = x::symbol_t::ELEMENT;
}

x::element_symbol::~element_symbol()
{
}

x::ast_ptr x::element_symbol::ast() const
{
	return element_ast;
}

x::element_decl_ast_ptr x::element_symbol::cast_ast() const
{
	return element_ast;
}

x::function_symbol::function_symbol()
{
	type = x::symbol_t::FUNCTION;
}

x::function_symbol::~function_symbol()
{
}

x::ast_ptr x::function_symbol::ast() const
{
	return function_ast;
}

x::function_decl_ast_ptr x::function_symbol::cast_ast() const
{
	return function_ast;
}

void x::function_symbol::add_child( x::symbol * val )
{
	XTHROW( x::semantic_exception, val->type == x::symbol_t::PARAM, "" );

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

x::ast_ptr x::variable_symbol::ast() const
{
	return variable_ast;
}

x::variable_decl_ast_ptr x::variable_symbol::cast_ast() const
{
	return variable_ast;
}

x::template_symbol::template_symbol()
{
	type = x::symbol_t::TEMPLATE;
}

x::template_symbol::~template_symbol()
{
}

x::ast_ptr x::template_symbol::ast() const
{
	return template_ast;
}

x::template_decl_ast_ptr x::template_symbol::cast_ast() const
{
	return template_ast;
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
		XTHROW( x::semantic_exception, false, "" );
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

x::ast_ptr x::namespace_symbol::ast() const
{
	return namespace_ast;
}

x::namespace_decl_ast_ptr x::namespace_symbol::cast_ast() const
{
	return namespace_ast;
}

void x::namespace_symbol::add_child( x::symbol * val )
{
	XTHROW( x::semantic_exception, val->is_type(), "" );

	children.push_back( val->cast_type() );
}

x::symbol * x::namespace_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val ) { return val->cast_symbol()->name == name; } );
	return ( it != children.end() ) ? (*it)->cast_symbol() : nullptr;
}

x::foundation_symbol::foundation_symbol()
{
	type = x::symbol_t::FOUNDATION;
}

x::foundation_symbol::~foundation_symbol()
{
}

x::uint64 x::foundation_symbol::size() const
{
	return sz;
}

void x::foundation_symbol::add_child( x::symbol * val )
{
	switch ( val->type )
	{
	case x::symbol_t::FUNCTION:
		functions.push_back( static_cast<x::function_symbol *>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( static_cast<x::variable_symbol *>( val ) );
		break;
	default:
		XTHROW( x::semantic_exception, false, "" );
		break;
	}
}

x::symbol * x::foundation_symbol::find_child( std::string_view name ) const
{
	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val ) { return val->name == name; } );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val ) { return val->name == name; } );
	if ( fit != functions.end() )
		return *fit;

	return nullptr;
}

x::nativefunc_symbol::nativefunc_symbol()
{
	type = x::symbol_t::NATIVEFUNC;
}

x::nativefunc_symbol::~nativefunc_symbol()
{
}

void x::nativefunc_symbol::add_child( x::symbol * val )
{
}

x::symbol * x::nativefunc_symbol::find_child( std::string_view name ) const
{
	return nullptr;
}

x::builtinfunc_symbol::builtinfunc_symbol()
{
	type = x::symbol_t::BUILTINFUNC;
}

x::builtinfunc_symbol::~builtinfunc_symbol()
{
}

void x::builtinfunc_symbol::add_child( x::symbol * val )
{
}

x::symbol * x::builtinfunc_symbol::find_child( std::string_view name ) const
{
	return nullptr;
}

x::ast_ptr x::builtinfunc_symbol::transferred( const x::symbols_ptr & symbols, const x::ast_ptr & val ) const
{
	return built->transferred( symbols, val );
}

x::symbols::symbols()
{
	auto val = new namespace_symbol;
	_symbolmap[""] = val;
	_scope.push_back( val );
}

x::symbols::~symbols()
{
	for ( auto it : _symbolmap )
		delete it.second;
}

x::unit_symbol * x::symbols::add_unit( x::unit_ast * ast )
{
	std::string fullname = { ast->location.file.data(), ast->location.file.size() };

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new unit_symbol;

	sym->name = fullname;
	sym->fullname = fullname;
	sym->unit_ast = std::static_pointer_cast<x::unit_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::enum_symbol * x::symbols::add_enum( x::enum_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new enum_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->enum_ast = std::static_pointer_cast<x::enum_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::alias_symbol * x::symbols::add_alias( x::using_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new alias_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->using_ast = std::static_pointer_cast<x::using_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::class_symbol * x::symbols::add_class( x::class_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new class_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->class_ast = std::static_pointer_cast<x::class_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::block_symbol * x::symbols::add_block( x::compound_stat_ast * ast )
{
	std::string fullname = std::format( "block_{}_{}_{}", ast->location.file, ast->location.line, ast->location.col );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new block_symbol;

	sym->name = fullname;
	sym->fullname = fullname;
	sym->compound_ast = std::static_pointer_cast<x::compound_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::cycle_symbol * x::symbols::add_cycle( x::cycle_stat_ast * ast )
{
	std::string fullname = std::format( "cycle_{}_{}_{}", ast->location.file, ast->location.line, ast->location.col );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new cycle_symbol;

	sym->name = fullname;
	sym->fullname = fullname;
	sym->cycle_ast = std::static_pointer_cast<x::cycle_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::local_symbol * x::symbols::add_local( x::local_stat_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new local_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->local_ast = std::static_pointer_cast<x::local_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::param_symbol * x::symbols::add_param( x::parameter_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new param_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->parameter_ast = std::static_pointer_cast<x::parameter_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::element_symbol * x::symbols::add_element( x::element_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new element_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->element_ast = std::static_pointer_cast<x::element_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::function_symbol * x::symbols::add_function( x::function_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new function_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->function_ast = std::static_pointer_cast<x::function_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::variable_symbol * x::symbols::add_variable( x::variable_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new variable_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->variable_ast = std::static_pointer_cast<x::variable_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::template_symbol * x::symbols::add_template( x::template_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new template_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->template_ast = std::static_pointer_cast<x::template_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::namespace_symbol * x::symbols::add_namespace( x::namespace_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new namespace_symbol;

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->namespace_ast = std::static_pointer_cast<x::namespace_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::foundation_symbol * x::symbols::add_foundation( std::string_view name, x::uint64 size )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new x::foundation_symbol;

	sym->sz = size;
	sym->name = name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::nativefunc_symbol * x::symbols::add_nativefunc( std::string_view name, void * callback )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new x::nativefunc_symbol;

	sym->name = name;
	sym->fullname = fullname;

	sym->callback = callback;

	add_symbol( sym );

	return sym;
}

x::builtinfunc_symbol * x::symbols::add_builtinfunc( std::string_view name, x::builtin_ptr builtin )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = new x::builtinfunc_symbol;

	sym->name = name;
	sym->fullname = fullname;

	sym->built = builtin;

	add_symbol( sym );

	return sym;
}

void x::symbols::push_scope( std::string_view name )
{
	auto sym = cur_scope()->find_child( name );
	XTHROW( x::semantic_exception, sym && sym->is_scope(), "" );
	_scope.push_back( sym->cast_scope() );
}

void x::symbols::push_scope( x::scope_symbol * symbol )
{
	_scope.push_back( symbol );
}

x::scope_symbol * x::symbols::cur_scope() const
{
	return _scope.back();
}

void x::symbols::pop_scope()
{
	_scope.pop_back();
}

x::type_symbol * x::symbols::find_type_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, sym != nullptr && sym->is_type(), "" );
	return sym->cast_type();
}

x::scope_symbol * x::symbols::find_scope_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, sym != nullptr && sym->is_scope(), "" );
	return sym->cast_scope();
}

x::class_symbol * x::symbols::find_class_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, sym != nullptr && sym->type == x::symbol_t::CLASS, "" );
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
		scope = cur_scope();

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
	auto scope = cur_scope();

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

x::namespace_symbol * x::symbols::global_namespace() const
{
	return reinterpret_cast<x::namespace_symbol *>( _symbolmap.at( "" ) );
}

std::string x::symbols::calc_fullname( std::string_view name ) const
{
	std::string fullname;

	if ( auto sym = cur_scope()->cast_symbol() )
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

void x::symbols::add_reference( x::ast * ast, std::string_view name, x::symbol * val )
{
	_referencemap[ast].emplace( name, val );
}

x::symbol * x::symbols::find_reference( x::ast * ast, std::string_view name ) const
{
	auto it = _referencemap.find( ast );
	if ( it != _referencemap.end() )
	{
		auto it2 = it->second.find( { name.begin(), name.end() } );
		if ( it2 != it->second.end() )
			return it2->second;
	}
	return nullptr;
}

void x::symbols::add_symbol( x::symbol * val )
{
	val->parent = cur_scope()->cast_symbol();

	_symbolmap[val->fullname] = val;

	if ( val->is_ast() )
		_astmap[val->cast_ast()->ast()] = val;

	if ( val->type == x::symbol_t::TEMPLATE )
		_templatemap.emplace( val->fullname, static_cast<x::template_symbol *>( val ) );

	cur_scope()->add_child( val );
}
