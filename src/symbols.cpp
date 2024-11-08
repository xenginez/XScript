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

x::using_symbol::using_symbol()
{
	type = x::symbol_t::USING;
}

x::using_symbol::~using_symbol()
{
}

bool x::using_symbol::is_type() const
{
	return true;
}

x::uint64 x::using_symbol::size() const
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
	case x::symbol_t::USING:
		usings.push_back( std::static_pointer_cast<x::using_symbol>( val ) );
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
	auto ait = std::find_if( usings.begin(), usings.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( ait != usings.end() )
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

void x::function_symbol::add_child( const x::symbol_ptr & val )
{
	if ( val->type == x::symbol_t::PARAMATER_ELEMENT )
		parameters.push_back( std::static_pointer_cast<x::paramater_symbol>( val ) );
	else if ( val->type == x::symbol_t::BLOCK )
		block = std::static_pointer_cast<x::block_symbol>( val );
	else
		results.push_back( val );
}

x::symbol_ptr x::function_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val ) { return val->name == name; } );
	if ( it != parameters.end() )
		return *it;
	
	if ( block->name == name )
		return block;

	return nullptr;
}

x::operator_symbol::operator_symbol()
{
	type = x::symbol_t::OPERATOR;
}

x::operator_symbol::~operator_symbol()
{
}

bool x::operator_symbol::is_scope() const
{
	return true;
}

void x::operator_symbol::add_child( const x::symbol_ptr & val )
{
	if ( val->type == x::symbol_t::PARAMATER_ELEMENT )
		parameters.push_back( std::static_pointer_cast<x::paramater_symbol>( val ) );
	else if ( val->type == x::symbol_t::BLOCK )
		block = std::static_pointer_cast<x::block_symbol>( val );
	else
		results.push_back( val );
}

x::symbol_ptr x::operator_symbol::find_child( std::string_view name ) const
{
	auto it = std::find_if( parameters.begin(), parameters.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( it != parameters.end() )
		return *it;

	if ( block->name == name )
		return block;

	return nullptr;
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

x::uint64 x::template_symbol::size() const
{
	return x::uint64();
}

void x::template_symbol::add_child( const x::symbol_ptr & val )
{
	switch ( val->type )
	{
	case x::symbol_t::USING:
		usings.push_back( std::static_pointer_cast<x::using_symbol>( val ) );
		break;
	case x::symbol_t::VARIABLE:
		variables.push_back( std::static_pointer_cast<x::variable_symbol>( val ) );
		break;
	case x::symbol_t::FUNCTION:
		functions.push_back( std::static_pointer_cast<x::function_symbol>( val ) );
		break;
	default:
		XTHROW( x::semantic_exception, true, "" );
		break;
	}
}

x::symbol_ptr x::template_symbol::find_child( std::string_view name ) const
{
	auto ait = std::find_if( usings.begin(), usings.end(), [name]( auto val )
	{
		return val->name == name;
	} );
	if ( ait != usings.end() )
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

x::interface_symbol::interface_symbol()
{
	type = x::symbol_t::INTERFACE;
}

x::interface_symbol::~interface_symbol()
{

}

bool x::interface_symbol::is_type() const
{
	return true;
}

bool x::interface_symbol::is_scope() const
{
	return false;
}

void x::interface_symbol::add_child( const x::symbol_ptr & val )
{
	functions.push_back( std::static_pointer_cast<x::function_symbol>( val ) );
}

x::symbol_ptr x::interface_symbol::find_child( std::string_view name ) const
{
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

void x::namespace_symbol::add_child( const x::symbol_ptr & val )
{
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

void x::nativefunc_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::PARAMATER_ELEMENT, "" );

	parameters.push_back( std::static_pointer_cast<x::paramater_symbol>( val ) );
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

void x::builtinfunc_symbol::add_child( const x::symbol_ptr & val )
{
	XTHROW( x::semantic_exception, val->type != x::symbol_t::PARAMATER_ELEMENT, "" );

	parameters.push_back( std::static_pointer_cast<x::paramater_symbol>( val ) );
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

x::paramater_symbol::paramater_symbol()
{
	type = x::symbol_t::PARAMATER_ELEMENT;
}

x::paramater_symbol::~paramater_symbol()
{
}

bool x::paramater_symbol::is_value() const
{
	return true;
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

void x::symbols::add_module( x::module * val )
{

}

x::unit_symbol_ptr x::symbols::add_unit( const x::unit_ast_ptr & ast )
{
	std::string fullname = { ast->get_location().file.data(), ast->get_location().file.size() };

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<unit_symbol>();

	sym->name = fullname;
	sym->fullname = fullname;
	sym->access = x::access_t::PUBLIC;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::enum_symbol_ptr x::symbols::add_enum( const x::enum_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<enum_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::using_symbol_ptr x::symbols::add_using( const x::using_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<using_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::class_symbol_ptr x::symbols::add_class( const x::class_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<class_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::block_symbol_ptr x::symbols::add_block( const x::compound_stat_ast_ptr & ast )
{
	std::string name = std::format( "block_{}_{}_{}", x::hash( ast->get_location().file ), ast->get_location().line, ast->get_location().col );
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<block_symbol>();

	sym->name = name;
	sym->fullname = fullname;
	sym->access = x::access_t::PRIVATE;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::cycle_symbol_ptr x::symbols::add_cycle( const x::cycle_stat_ast_ptr & ast )
{
	std::string name = std::format( "cycle_{}_{}_{}", x::hash( ast->get_location().file ), ast->get_location().line, ast->get_location().col );
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<cycle_symbol>();

	sym->name = name;
	sym->fullname = fullname;
	sym->access = x::access_t::PRIVATE;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::local_symbol_ptr x::symbols::add_local( const x::local_stat_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<local_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = x::access_t::PRIVATE;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::function_symbol_ptr x::symbols::add_function( const x::function_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<function_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::function_symbol_ptr x::symbols::add_operator( const x::operator_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<function_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::variable_symbol_ptr x::symbols::add_variable( const x::variable_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<variable_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::template_symbol_ptr x::symbols::add_template( const x::template_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<template_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::paramater_symbol_ptr x::symbols::add_paramater( const x::parameter_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<paramater_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = x::access_t::PRIVATE;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::interface_symbol_ptr x::symbols::add_interface( const x::interface_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<interface_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

x::namespace_symbol_ptr x::symbols::add_namespace( const x::namespace_decl_ast_ptr & ast )
{
	std::string fullname = calc_fullname( ast->get_name() );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<namespace_symbol>();

	sym->name = ast->get_name();
	sym->fullname = fullname;
	sym->access = ast->get_access();

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

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
	sym->access = x::access_t::PUBLIC;

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
	sym->access = x::access_t::PUBLIC;

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
	sym->access = x::access_t::PUBLIC;

	add_symbol( sym );

	return sym;
}

x::enum_element_symbol_ptr x::symbols::add_enum_element( std::string_view name, const x::expr_stat_ast_ptr & ast )
{
	std::string fullname = calc_fullname( name );

	XTHROW( x::semantic_exception, _symbolmap.find( fullname ) != _symbolmap.end(), "" );

	auto sym = std::make_shared<enum_element_symbol>();

	sym->name = name;
	sym->fullname = fullname;
	sym->access = x::access_t::PUBLIC;

	add_symbol( sym );
	_astsymmap[ast] = sym;
	_symastmap[sym] = ast;

	return sym;
}

void x::symbols::push_scope( std::string_view name )
{
	auto sym = current_scope()->find_child( name );
	XTHROW( x::semantic_exception, !sym || !sym->is_scope(), "" );
	_scope.push_back( sym );
}

void x::symbols::push_scope( const x::symbol_ptr & symbol )
{
	_scope.push_back( symbol );
}

x::symbol_ptr x::symbols::current_scope() const
{
	return _scope.back();
}

x::symbol_ptr x::symbols::pop_scope()
{
	x::symbol_ptr back;
	if ( !_scope.empty() )
	{
		back = _scope.back();

		_scope.pop_back();
	}
	return back;
}

x::ast_ptr x::symbols::find_ast( const x::symbol_ptr & val ) const
{
	auto it = _symastmap.find( val );
	
	return it != _symastmap.end() ? it->second : nullptr;
}

x::ast_ptr x::symbols::find_ast( x::symbol_t type, x::symbol_ptr scope ) const
{
	return find_ast( find_symbol( type, scope ) );
}

x::ast_ptr x::symbols::find_ast( std::string_view name, x::symbol_ptr scope ) const
{
	return find_ast( find_symbol( name, scope ) );
}

x::ast_ptr x::symbols::find_ast( const x::type_ast_ptr & ast, x::symbol_ptr scope ) const
{
	return find_ast( find_symbol( ast, scope ) );
}

x::ast_ptr x::symbols::find_ast( std::string_view name, x::symbol_t type, x::symbol_ptr scope ) const
{
	return find_ast( find_symbol( name, type, scope ) );
}

x::symbol_ptr x::symbols::find_symbol( x::symbol_t type, x::symbol_ptr scope ) const
{
	if ( scope == nullptr )
		scope = current_scope();

	if ( scope->type == type )
		return scope;

	if ( auto sym = up_find_symbol( type, scope ) )
		return sym;

	return nullptr;
}

x::symbol_ptr x::symbols::find_symbol( std::string_view name, x::symbol_ptr scope ) const
{
	if ( scope == nullptr )
		scope = current_scope();

	if ( name.empty() )
		return nullptr;

	if ( scope->name == name )
		return scope;

	if ( auto sym = up_find_symbol( name, scope ) )
		return sym;

	if ( auto sym = down_find_symbol( name, scope ) )
		return sym;
	
	return nullptr;
}

x::symbol_ptr x::symbols::find_symbol( const x::type_ast_ptr & ast, x::symbol_ptr scope ) const
{
	return {};
}

x::symbol_ptr x::symbols::find_symbol( std::string_view name, x::symbol_t type, x::symbol_ptr scope ) const
{
	if ( scope == nullptr )
		scope = current_scope();

	if ( name.empty() )
		return nullptr;

	if ( scope->name == name && scope->type == type )
		return scope;
	
	if ( auto sym = up_find_symbol( name, scope ) )
	{
		if ( sym->type == type )
			return sym;
	}

	if ( auto sym = down_find_symbol( name, scope ) )
	{
		if( sym->type == type )
		return sym;
	}

	return nullptr;
}

x::symbol_ptr x::symbols::up_find_symbol( x::symbol_t type, const x::symbol_ptr & scope ) const
{
	if ( auto parent = scope->parent.lock() )
	{
		if ( parent == nullptr )
			return nullptr;

		if ( parent->type == type )
		{
			return parent;
		}
		else
		{
			return up_find_symbol( type, parent );
		}
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

	if ( auto sym = current_scope() )
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
	auto it = _astsymmap.find( ast );
	return it != _astsymmap.end() ? it->second : nullptr;
}

x::symbol_ptr x::symbols::find_symbol_from_fullname( std::string_view fullname ) const
{
	auto it = _symbolmap.find( { fullname.begin(), fullname.end() } );

	return it != _symbolmap.end() ? it->second : nullptr;
}

std::vector<x::template_symbol_ptr> x::symbols::find_template_symbols( std::string_view fullname ) const
{
	std::vector<x::template_symbol_ptr> result;

	auto pair = _templatemap.equal_range( { fullname.begin(), fullname.end() } );
	for ( auto it = pair.first; it != pair.second; ++it )
	{
		result.push_back( it->second );
	}

	return result;
}

void x::symbols::add_reference( const x::ast_ptr & ast, std::string_view name, const x::symbol_ptr & val )
{
	_referencemap[ast].emplace( name, val );
}

x::symbol_ptr x::symbols::find_reference( const x::ast_ptr & ast, std::string_view name ) const
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
	val->parent = current_scope();
	val->symbols = shared_from_this();

	_symbolmap[val->fullname] = val;

	if ( val->type == x::symbol_t::TEMPLATE )
		_templatemap.emplace( val->fullname, std::static_pointer_cast<x::template_symbol>( val ) );

	current_scope()->add_child( val );
}
