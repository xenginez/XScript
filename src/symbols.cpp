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

bool x::symbol::is_type() const
{
	return false;
}

bool x::symbol::is_scope() const
{
	return false;
}

bool x::symbol::is_value() const
{
	return false;
}

x::ast_ptr x::symbol::ast() const
{
	return nullptr;
}

x::uint64 x::symbol::size() const
{
	return 0;
}

void x::symbol::add_child( const x::symbol_ptr & val )
{
}

x::symbol_ptr x::symbol::find_child( std::string_view name ) const
{
	return nullptr;
}

x::unit_symbol::unit_symbol()
{
	type = x::symbol_t::UNIT;
}

x::unit_symbol::~unit_symbol()
{
}

bool x::unit_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::unit_symbol::ast() const
{
	return unit_ast;
}

void x::unit_symbol::add_child( const x::symbol_ptr & val )
{
	children.push_back( val );
}

x::symbol_ptr x::unit_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != children.end() ) ? *it : nullptr;
}

x::enum_symbol::enum_symbol()
{
	type = x::symbol_t::ENUM;
}

x::enum_symbol::~enum_symbol()
{
}

bool x::enum_symbol::is_type() const
{
	return true;
}

bool x::enum_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::enum_symbol::ast() const
{
	return enum_ast;
}

x::uint64 x::enum_symbol::size() const
{
	return x::uint64();
}

void x::enum_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::ENUM_ELEMENT, "" );

	elements.push_back( std::static_pointer_cast<x::enum_element_symbol>( val ) );
}

x::symbol_ptr x::enum_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( elements.begin(), elements.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != elements.end() ) ? *it : nullptr;
}

x::alias_symbol::alias_symbol()
{
	type = x::symbol_t::ALIAS;
}

x::alias_symbol::~alias_symbol()
{
}

bool x::alias_symbol::is_type() const
{
	return true;
}

x::ast_ptr x::alias_symbol::ast() const
{
	return using_ast;
}

x::uint64 x::alias_symbol::size() const
{
	return retype.lock()->size();
}

x::class_symbol::class_symbol()
{
	type = x::symbol_t::CLASS;
}

x::class_symbol::~class_symbol()
{
}

bool x::class_symbol::is_type() const
{
	return true;
}

bool x::class_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::class_symbol::ast() const
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

void x::class_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::ALIAS:
		aliases.push_back( std::static_pointer_cast<x::alias_symbol>( val ) );
		break;
	case x::symbol_t::FUNCTION:
		functions.push_back( std::static_pointer_cast<x::function_symbol>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( std::static_pointer_cast<x::variable_symbol>( val ) );
		break;
	default:
		XTHROW( x::semantic_exception, true, "" );
		break;
	}
}

x::symbol_ptr x::class_symbol::find_child( std::string_view name ) const
{
	auto ait = std::find_if( aliases.begin(), aliases.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( ait != aliases.end() )
		return *ait;

	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val )
	{
		return val->name == name;
	} );
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

bool x::block_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::block_symbol::ast() const
{
	return compound_ast;
}

void x::block_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::LOCAL:
		locals.push_back( std::static_pointer_cast<x::local_symbol>( val ) );
		break;
	case x::symbol_t::BLOCK:
	case x::symbol_t::CYCLE:
		blocks.push_back( std::static_pointer_cast<x::block_symbol>( val ) );
		break;
	}
}

x::symbol_ptr x::block_symbol::find_child( std::string_view name ) const
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

x::cycle_symbol::cycle_symbol()
{
	type = x::symbol_t::CYCLE;
}

x::cycle_symbol::~cycle_symbol()
{
}

bool x::cycle_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::cycle_symbol::ast() const
{
	return cycle_ast;
}

void x::cycle_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::LOCAL:
		locals.push_back( std::static_pointer_cast<x::local_symbol>( val ) );
		break;
	case x::symbol_t::BLOCK:
	case x::symbol_t::CYCLE:
		blocks.push_back( std::static_pointer_cast<x::block_symbol>( val ) );
		break;
	}
}

x::symbol_ptr x::cycle_symbol::find_child( std::string_view name ) const
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

bool x::local_symbol::is_value() const
{
	return true;
}

x::ast_ptr x::local_symbol::ast() const
{
	return local_ast;
}

x::function_symbol::function_symbol()
{
	type = x::symbol_t::FUNCTION;
}

x::function_symbol::~function_symbol()
{
}

bool x::function_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::function_symbol::ast() const
{
	return function_ast;
}

void x::function_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::PARAMATER_ELEMENT, "" );

	parameters.push_back( std::static_pointer_cast<x::paramater_element_symbol>( val ) );
}

x::symbol_ptr x::function_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != parameters.end() ) ? *it : nullptr;
}

x::variable_symbol::variable_symbol()
{
	type = x::symbol_t::VARIABLE;
}

x::variable_symbol::~variable_symbol()
{
}

bool x::variable_symbol::is_value() const
{
	return true;
}

x::ast_ptr x::variable_symbol::ast() const
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

bool x::template_symbol::is_type() const
{
	return true;
}

bool x::template_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::template_symbol::ast() const
{
	return template_ast;
}

x::uint64 x::template_symbol::size() const
{
	return x::uint64();
}

void x::template_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::ALIAS:
		aliases.push_back( std::static_pointer_cast<x::alias_symbol>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( std::static_pointer_cast<x::variable_symbol>( val ) );
		break;
	case x::symbol_t::FUNCTION:
		functions.push_back( std::static_pointer_cast<x::function_symbol>( val ) );
		break;
	case x::symbol_t::TEMPLATE_ELEMENT:
		elements.push_back( std::static_pointer_cast<x::template_element_symbol>( val ) );
		break;
	default:
		XTHROW( x::semantic_exception, true, "" );
		break;
	}
}

x::symbol_ptr x::template_symbol::find_child( std::string_view name ) const
{
	auto ait = std::find_if( aliases.begin(), aliases.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( ait != aliases.end() )
		return *ait;

	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val )
	{
		return val->name == name;
	} );
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

bool x::namespace_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::namespace_symbol::ast() const
{
	return namespace_ast;
}

void x::namespace_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, !val->is_type(), "" );

	children.push_back( val );
}

x::symbol_ptr x::namespace_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( children.begin(), children.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != children.end() ) ? ( *it ) : nullptr;
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

bool x::foundation_symbol::is_type() const
{
	return true;
}

bool x::foundation_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::foundation_symbol::ast() const
{
	return nullptr;
}

void x::foundation_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::FUNCTION:
		functions.push_back( std::static_pointer_cast<x::function_symbol>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( std::static_pointer_cast<x::variable_symbol>( val ) );
		break;
	default:
		XTHROW( x::semantic_exception, true, "" );
		break;
	}
}

x::symbol_ptr x::foundation_symbol::find_child( std::string_view name ) const
{
	auto vit = std::find_if( variables.begin(), variables.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( vit != variables.end() )
		return *vit;

	auto fit = std::find_if( functions.begin(), functions.end(), [name]( auto val )
	{
		return val->name == name;
	} );
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

bool x::nativefunc_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::nativefunc_symbol::ast() const
{
	return nullptr;
}

void x::nativefunc_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::PARAMATER_ELEMENT, "" );

	parameters.push_back( std::static_pointer_cast<x::paramater_element_symbol>( val ) );
}

x::symbol_ptr x::nativefunc_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != parameters.end() ) ? *it : nullptr;
}

x::builtinfunc_symbol::builtinfunc_symbol()
{
	type = x::symbol_t::BUILTINFUNC;
}

x::builtinfunc_symbol::~builtinfunc_symbol()
{
}

bool x::builtinfunc_symbol::is_scope() const
{
	return true;
}

x::ast_ptr x::builtinfunc_symbol::ast() const
{
	return nullptr;
}

void x::builtinfunc_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::PARAMATER_ELEMENT, "" );

	parameters.push_back( std::static_pointer_cast<x::paramater_element_symbol>( val ) );
}

x::symbol_ptr x::builtinfunc_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	return ( it != parameters.end() ) ? *it : nullptr;
}

x::enum_element_symbol::enum_element_symbol()
{
	type = x::symbol_t::ENUM_ELEMENT;
}

x::enum_element_symbol::~enum_element_symbol()
{
}

bool x::enum_element_symbol::is_value() const
{
	return true;
}

x::ast_ptr x::enum_element_symbol::ast() const
{
	return element_ast;
}

x::template_element_symbol::template_element_symbol()
{
	type = x::symbol_t::TEMPLATE_ELEMENT;
}

x::template_element_symbol::~template_element_symbol()
{
}

bool x::template_element_symbol::is_type() const
{
	return true;
}

x::ast_ptr x::template_element_symbol::ast() const
{
	return template_ast;
}

x::paramater_element_symbol::paramater_element_symbol()
{
	type = x::symbol_t::PARAMATER_ELEMENT;
}

x::paramater_element_symbol::~paramater_element_symbol()
{
}

bool x::paramater_element_symbol::is_value() const
{
	return true;
}

x::ast_ptr x::paramater_element_symbol::ast() const
{
	return parameter_ast;
}

x::symbols::symbols()
{
	auto val = std::make_shared<namespace_symbol>();
	_symbolmap[""] = val;
	_scope.push_back( val );
}

x::symbols::~symbols()
{
}

x::unit_symbol_ptr x::symbols::add_unit( x::unit_ast * ast )
{
	std::string fullname = { ast->location.file.data(), ast->location.file.size() };

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<unit_symbol>();

	sym->name = fullname;
	sym->fullname = fullname;
	sym->unit_ast = std::static_pointer_cast<x::unit_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::enum_element_symbol_ptr x::symbols::add_enum_elem( x::enum_element_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<enum_element_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->element_ast = std::static_pointer_cast<x::enum_element_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::template_element_symbol_ptr x::symbols::add_template_elem( x::template_element_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<template_element_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->is_multi = ast->is_multi;
	sym->template_ast = std::static_pointer_cast<x::template_element_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::paramater_element_symbol_ptr x::symbols::add_paramater_elem( x::parameter_element_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<paramater_element_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->parameter_ast = std::static_pointer_cast<x::parameter_element_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::enum_symbol_ptr x::symbols::add_enum( x::enum_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<enum_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->enum_ast = std::static_pointer_cast<x::enum_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::alias_symbol_ptr x::symbols::add_alias( x::using_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<alias_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->using_ast = std::static_pointer_cast<x::using_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::class_symbol_ptr x::symbols::add_class( x::class_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<class_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->class_ast = std::static_pointer_cast<x::class_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::block_symbol_ptr x::symbols::add_block( x::compound_stat_ast * ast )
{
	std::string fullname = std::format( "block_{}_{}_{}", x::hash( ast->location.file ), ast->location.line, ast->location.col );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<block_symbol>();

	sym->name = fullname;
	sym->fullname = fullname;
	sym->compound_ast = std::static_pointer_cast<x::compound_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::cycle_symbol_ptr x::symbols::add_cycle( x::cycle_stat_ast * ast )
{
	std::string fullname = std::format( "cycle_{}_{}_{}", x::hash( ast->location.file ), ast->location.line, ast->location.col );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<cycle_symbol>();

	sym->name = fullname;
	sym->fullname = fullname;
	sym->cycle_ast = std::static_pointer_cast<x::cycle_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::local_symbol_ptr x::symbols::add_local( x::local_stat_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<local_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->local_ast = std::static_pointer_cast<x::local_stat_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::function_symbol_ptr x::symbols::add_function( x::function_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<function_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->function_ast = std::static_pointer_cast<x::function_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::variable_symbol_ptr x::symbols::add_variable( x::variable_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<variable_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->variable_ast = std::static_pointer_cast<x::variable_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::template_symbol_ptr x::symbols::add_template( x::template_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<template_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->template_ast = std::static_pointer_cast<x::template_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::namespace_symbol_ptr x::symbols::add_namespace( x::namespace_decl_ast * ast )
{
	std::string fullname = calc_fullname( ast->name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<namespace_symbol>();

	sym->name = ast->name;
	sym->fullname = fullname;
	sym->namespace_ast = std::static_pointer_cast<x::namespace_decl_ast>( ast->shared_from_this() );

	add_symbol( sym );

	return sym;
}

x::foundation_symbol_ptr x::symbols::add_foundation( std::string_view name, x::uint64 size )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<x::foundation_symbol>();

	sym->sz = size;
	sym->name = name;
	sym->fullname = fullname;

	add_symbol( sym );

	return sym;
}

x::nativefunc_symbol_ptr x::symbols::add_nativefunc( std::string_view name, void * callback )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<x::nativefunc_symbol>();

	sym->name = name;
	sym->fullname = fullname;
	sym->callback = callback;

	add_symbol( sym );

	return sym;
}

x::builtinfunc_symbol_ptr x::symbols::add_builtinfunc( std::string_view name, x::builtin_ptr builtin )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<x::builtinfunc_symbol>();

	sym->name = name;
	sym->fullname = fullname;
	sym->built = builtin;

	add_symbol( sym );

	return sym;
}

void x::symbols::push_scope( std::string_view name )
{
	auto sym = cur_scope()->find_child( name );
	XTHROW( x::semantic_exception, !sym || !sym->is_scope(), "" );
	_scope.push_back( sym );
}

void x::symbols::push_scope( const x::symbol_ptr & symbol )
{
	_scope.push_back( symbol );
}

x::symbol_ptr x::symbols::cur_scope() const
{
	return _scope.back();
}

void x::symbols::pop_scope()
{
	_scope.pop_back();
}

x::symbol_ptr x::symbols::find_type_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, !sym || !sym->is_type(), "" );
	return sym;
}

x::symbol_ptr x::symbols::find_scope_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, !sym || !sym->is_scope(), "" );
	return sym;
}

x::symbol_ptr x::symbols::find_class_symbol( std::string_view name ) const
{
	auto sym = find_symbol( name );
	XTHROW( x::semantic_exception, !sym || sym->type != x::symbol_t::CLASS, "" );
	return sym;
}

std::vector<x::template_symbol_ptr> x::symbols::find_template_symbols( std::string_view name ) const
{
	std::vector<x::template_symbol_ptr> result;

	auto pair = _templatemap.equal_range( calc_fullname( name ) );
	for ( auto it = pair.first; it != pair.second; ++it )
	{
		result.push_back( it->second );
	}

	return result;
}

x::symbol_ptr x::symbols::find_symbol( std::string_view name, x::symbol_ptr scope ) const
{
	if ( scope == nullptr )
		scope = cur_scope();

	if ( name.empty() )
		return scope;

	if ( scope->name == name )
		return scope;

	if ( auto sym = down_find_symbol( name, scope ) )
		return sym;
	else if ( auto sym = up_find_symbol( name, scope ) )
		return sym;

	return nullptr;
}

x::symbol_ptr x::symbols::up_find_symbol_from_type( x::symbol_t type ) const
{
	auto scope = cur_scope();

	while ( scope )
	{
		if ( scope->type == type )
			return scope;

		auto parent = scope->parent.lock();
		if ( parent && parent->is_scope() )
			scope = parent;
		else
			scope = nullptr;
	}

	return nullptr;
}

x::symbol_ptr x::symbols::up_find_symbol( std::string_view name, const x::symbol_ptr & scope ) const
{
	auto beg = name_beg( name );
	auto end = name_end( name );
	std::string_view nname{ beg, end };

	if ( auto parent = scope->parent.lock() )
	{
		if ( parent->name == nname )
		{
			if ( end == name.end() )
				return parent;
			else
				return down_find_symbol( { end + 1, name_end( name, end + 1 ) }, parent );
		}
		else
		{
			return up_find_symbol( name, parent );
		}
	}

	return nullptr;
}

x::symbol_ptr x::symbols::down_find_symbol( std::string_view name, const x::symbol_ptr & scope ) const
{
	auto beg = name_beg( name );
	auto end = name_end( name );
	std::string_view nname{ beg, end };

	if ( auto child = scope->find_child( nname ) )
	{
		if ( end == name.end() )
			return child;
		else if ( child->is_scope() )
			return down_find_symbol( { end + 1, name_end( name, end + 1 ) }, child );
	}

	return nullptr;
}

x::namespace_symbol_ptr x::symbols::global_namespace() const
{
	return std::dynamic_pointer_cast<x::namespace_symbol>( _symbolmap.at( "" ) );
}

std::string x::symbols::calc_fullname( std::string_view name ) const
{
	std::string fullname;

	if ( auto sym = cur_scope() )
	{
		if ( sym->type == x::symbol_t::UNIT || ( sym->type == x::symbol_t::NAMESPACE && sym->name.empty() ) )
			fullname = name;
		else
			fullname = std::format( "{}.{}", sym->fullname, name );
	}

	return fullname;
}

x::symbol_ptr x::symbols::find_symbol_from_ast( const x::ast_ptr & ast ) const
{
	auto it = _astmap.find( ast );
	return it != _astmap.end() ? it->second : nullptr;
}

x::symbol_ptr x::symbols::find_symbol_from_fullname( std::string_view fullname ) const
{
	auto it = _symbolmap.find( { fullname.begin(), fullname.end() } );

	return it != _symbolmap.end() ? it->second : nullptr;
}

void x::symbols::add_reference( x::ast * ast, std::string_view name, const x::symbol_ptr & val )
{
	_referencemap[ast].emplace( name, val );
}

x::symbol_ptr x::symbols::find_reference( x::ast * ast, std::string_view name ) const
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

void x::symbols::add_symbol( const x::symbol_ptr & val )
{
	val->parent = cur_scope();
	val->symbols = shared_from_this();

	_symbolmap[val->fullname] = val;

	if ( val->ast() )
		_astmap[val->ast()] = val;

	if ( val->type == x::symbol_t::TEMPLATE )
		_templatemap.emplace( val->fullname, std::static_pointer_cast<x::template_symbol>( val ) );

	cur_scope()->add_child( val );
}
