#include "ast.h"

#include "visitor.h"

#define RESETC( C ) \
for ( auto & it : C ) \
{ \
	if ( it == old_child ) \
	{ \
		if ( auto new_ptr = std::dynamic_pointer_cast<std::remove_reference_t<decltype( it )>::element_type>( new_child ) ) \
		{ \
			it = new_ptr; \
			return true; \
		} \
	} \
}

#define RESETCC( C ) \
for ( auto & it : C ) \
{ \
	if ( it.first == old_child ) \
	{ \
		if ( auto new_ptr = std::dynamic_pointer_cast<std::remove_reference_t<decltype( it.first )>::element_type>( new_child ) ) \
		{ \
			it.first = new_ptr; \
			return true; \
		} \
	} \
	if ( it.second == old_child ) \
	{ \
		if ( auto new_ptr = std::dynamic_pointer_cast<std::remove_reference_t<decltype( it.second )>::element_type>( new_child ) ) \
		{ \
			it.second = new_ptr; \
			return true; \
		} \
	} \
}

bool x::ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	return false;
}

x::ast_ptr x::ast::get_parent() const
{
	return _parent.lock();
}

void x::ast::set_parent( const x::ast_ptr & val )
{
	_parent = val;
}

const x::location & x::ast::get_location() const
{
	return _location;
}

void x::ast::set_location( const x::location & val )
{
	_location = val;
}

x::ast_t x::unit_ast::type() const
{
	return x::ast_t::UNIT;
}

void x::unit_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::unit_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _imports );
	RESETC( _namespaces );
	return false;
}

std::span<const x::import_ast_ptr> x::unit_ast::get_imports() const
{
	return _imports;
}

void x::unit_ast::insert_import( const x::import_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_imports.push_back( val );
}

std::span<const x::namespace_decl_ast_ptr> x::unit_ast::get_namespaces() const
{
	return _namespaces;
}

void x::unit_ast::insert_namespace( const x::namespace_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_namespaces.push_back( val );
}

x::ast_t x::import_ast::type() const
{
	return x::ast_t::IMPORT;
}

void x::import_ast::accept( visitor * val )
{
	val->visit( this );
}

const std::string & x::import_ast::get_path() const
{
	return _path;
}

void x::import_ast::set_path( const std::string & val )
{
	_path = val;
}

x::ast_t x::attribute_ast::type() const
{
	return x::ast_t::ATTRIBUTE;
}

void x::attribute_ast::accept( visitor * val )
{
	val->visit( this );
}

std::span<const x::attribute_ast::string_pair> x::attribute_ast::get_attributes() const
{
	return _attributes;
}

void x::attribute_ast::insert_attribute( const x::attribute_ast::string_pair & val )
{
	_attributes.push_back( val );
}

x::ast_t x::parameter_ast::type() const
{
	return x::ast_t::PARAMETER;
}

void x::parameter_ast::accept( visitor * val )
{
	val->visit( this );
}

const std::string & x::parameter_ast::get_name() const
{
	return _name;
}

void x::parameter_ast::set_name( const std::string & val )
{
	_name = val;
}

const x::type_ast_ptr & x::parameter_ast::get_valuetype() const
{
	return _valuetype;
}

void x::parameter_ast::set_valuetype( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_valuetype = val;
}

const x::expr_stat_ast_ptr & x::parameter_ast::get_default() const
{
	return _default;
}

void x::parameter_ast::set_default( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_default = val;
}

x::ast_t x::type_ast::type() const
{
	return x::ast_t::TYPE;
}

void x::type_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::type_ast::get_is_ref() const
{
	return _is_ref;
}

void x::type_ast::set_is_ref( bool val )
{
	_is_ref = val;
}

bool x::type_ast::get_is_const() const
{
	return _is_const;
}

void x::type_ast::set_is_const( bool val )
{
	_is_const = val;
}

const std::string & x::type_ast::get_name() const
{
	return _name;
}

void x::type_ast::set_name( const std::string & val )
{
	_name = val;
}

x::ast_t x::func_type_ast::type() const
{
	return x::ast_t::FUNC_TYPE;
}

void x::func_type_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::func_type_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _parameters );
	return false;
}

std::span<const x::type_ast_ptr> x::func_type_ast::get_parameters() const
{
	return _parameters;
}

void x::func_type_ast::insert_parameter( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_parameters.push_back( val );
}

x::ast_t x::temp_type_ast::type() const
{
	return x::ast_t::TEMP_TYPE;
}

void x::temp_type_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::temp_type_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _elements );

	return false;
}

std::span<const x::expr_stat_ast_ptr> x::temp_type_ast::get_elements() const
{
	return _elements;
}

void x::temp_type_ast::insert_element( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_elements.push_back( val );
}

x::ast_t x::list_type_ast::type() const
{
	return x::ast_t::LIST_TYPE;
}

void x::list_type_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::array_type_ast::type() const
{
	return x::ast_t::ARRAY_TYPE;
}

void x::array_type_ast::accept( visitor * val )
{
	val->visit( this );
}

int x::array_type_ast::get_layer() const
{
	return _layer;
}

void x::array_type_ast::set_layer( int val )
{
	_layer = val;
}

const std::string & x::decl_ast::get_name() const
{
	return _name;
}

void x::decl_ast::set_name( const std::string & val )
{
	_name = val;
}

x::access_t x::decl_ast::get_access() const
{
	return _access;
}

void x::decl_ast::set_access( x::access_t val )
{
	_access = val;
}

const x::attribute_ast_ptr & x::decl_ast::get_attribute() const
{
	return _attribute;
}

void x::decl_ast::set_attribute( const x::attribute_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_attribute = val;
}

x::ast_t x::enum_decl_ast::type() const
{
	return x::ast_t::ENUM_DECL;
}

void x::enum_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

std::span<const x::enum_decl_ast::element_pair> x::enum_decl_ast::get_elements() const
{
	return _elements;
}

void x::enum_decl_ast::insert_element( const element_pair & val )
{
	val.second->set_parent( shared_from_this() );

	_elements.push_back( val );
}

x::ast_t x::class_decl_ast::type() const
{
	return x::ast_t::CLASS_DECL;
}

void x::class_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::class_decl_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _interfaces );
	RESETC( _usings );
	RESETC( _variables );
	RESETC( _functions );

	return false;
}

const x::type_ast_ptr & x::class_decl_ast::get_base() const
{
	return _base;
}

void x::class_decl_ast::set_base( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_base = val;
}

const x::function_decl_ast_ptr & x::class_decl_ast::get_construct() const
{
	return _construct;
}

void x::class_decl_ast::set_construct( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_construct = val;
}

const x::function_decl_ast_ptr & x::class_decl_ast::get_finalize() const
{
	return _finalize;
}

void x::class_decl_ast::set_finalize( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_finalize = val;
}

std::span<const x::type_ast_ptr> x::class_decl_ast::get_friends() const
{
	return _friends;
}

void x::class_decl_ast::insert_friend( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_friends.push_back( val );
}

std::span<const x::type_ast_ptr> x::class_decl_ast::get_interfaces() const
{
	return _interfaces;
}

void x::class_decl_ast::insert_interface( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_interfaces.push_back( val );
}

std::span<const x::using_decl_ast_ptr> x::class_decl_ast::get_usings() const
{
	return _usings;
}

void x::class_decl_ast::insert_using( const x::using_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_usings.push_back( val );
}

std::span<const x::variable_decl_ast_ptr> x::class_decl_ast::get_variables() const
{
	return _variables;
}

void x::class_decl_ast::insert_variable( const x::variable_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_variables.push_back( val );
}

std::span<const x::function_decl_ast_ptr> x::class_decl_ast::get_functions() const
{
	return _functions;
}

void x::class_decl_ast::insert_function( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_functions.push_back( val );
}

x::ast_t x::using_decl_ast::type() const
{
	return x::ast_t::USING_DECL;
}

void x::using_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::type_ast_ptr & x::using_decl_ast::get_retype() const
{
	return _retype;
}

void x::using_decl_ast::set_retype( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_retype = val;
}

x::ast_t x::template_decl_ast::type() const
{
	return x::ast_t::TEMPLATE_DECL;
}

void x::template_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::template_decl_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _interfaces );
	RESETC( _usings );
	RESETC( _variables );
	RESETC( _functions );
	RESETC( _elements );

	return false;
}

const x::type_ast_ptr & x::template_decl_ast::get_base() const
{
	return _base;
}

void x::template_decl_ast::set_base( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_base = val;
}

const x::compound_stat_ast_ptr & x::template_decl_ast::get_where() const
{
	return _where;
}

void x::template_decl_ast::set_where( const x::compound_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_where = val;
}

const x::function_decl_ast_ptr & x::template_decl_ast::get_construct() const
{
	return _construct;
}

void x::template_decl_ast::set_construct( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_construct = val;
}

const x::function_decl_ast_ptr & x::template_decl_ast::get_finalize() const
{
	return _finalize;
}

void x::template_decl_ast::set_finalize( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_finalize = val;
}

std::span<const x::type_ast_ptr> x::template_decl_ast::get_friends() const
{
	return _friends;
}

void x::template_decl_ast::insert_friend( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_friends.push_back( val );
}

std::span<const x::parameter_ast_ptr> x::template_decl_ast::get_elements() const
{
	return _elements;
}

void x::template_decl_ast::insert_element( const x::parameter_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_elements.push_back( val );
}

std::span<const x::type_ast_ptr> x::template_decl_ast::get_interfaces() const
{
	return _interfaces;
}

void x::template_decl_ast::insert_interface( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_interfaces.push_back( val );
}

std::span<const x::using_decl_ast_ptr> x::template_decl_ast::get_usings() const
{
	return _usings;
}

void x::template_decl_ast::insert_using( const x::using_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_usings.push_back( val );
}

std::span<const x::variable_decl_ast_ptr> x::template_decl_ast::get_variables() const
{
	return _variables;
}

void x::template_decl_ast::insert_variable( const x::variable_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_variables.push_back( val );
}

std::span<const x::function_decl_ast_ptr> x::template_decl_ast::get_functions() const
{
	return _functions;
}

void x::template_decl_ast::insert_function( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_functions.push_back( val );
}

x::ast_t x::variable_decl_ast::type() const
{
	return x::ast_t::VARIABLE_DECL;
}

void x::variable_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::variable_decl_ast::get_is_local() const
{
	return _is_local;
}

void x::variable_decl_ast::set_is_local( bool val )
{
	_is_local = val;
}

bool x::variable_decl_ast::get_is_static() const
{
	return _is_static;
}

void x::variable_decl_ast::set_is_static( bool val )
{
	_is_static = val;
}

bool x::variable_decl_ast::get_is_thread() const
{
	return _is_thread;
}

void x::variable_decl_ast::set_is_thread( bool val )
{
	_is_thread = val;
}

const x::type_ast_ptr & x::variable_decl_ast::get_valuetype() const
{
	return _valuetype;
}

void x::variable_decl_ast::set_valuetype( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_valuetype = val;
}

const x::initializer_expr_ast_ptr & x::variable_decl_ast::get_init() const
{
	return _init;
}

void x::variable_decl_ast::set_init( const x::initializer_expr_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_init = val;
}

x::ast_t x::function_decl_ast::type() const
{
	return x::ast_t::FUNCTION_DECL;
}

void x::function_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::function_decl_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _results );
	RESETC( _parameters );

	return false;
}

bool x::function_decl_ast::get_is_const() const
{
	return _is_const;
}

void x::function_decl_ast::set_is_const( bool val )
{
	_is_const = val;
}

bool x::function_decl_ast::get_is_async() const
{
	return _is_async;
}

void x::function_decl_ast::set_is_async( bool val )
{
	_is_async = val;
}

bool x::function_decl_ast::get_is_final() const
{
	return _is_final;
}

void x::function_decl_ast::set_is_final( bool val )
{
	_is_final = val;
}

bool x::function_decl_ast::get_is_static() const
{
	return _is_static;
}

void x::function_decl_ast::set_is_static( bool val )
{
	_is_static = val;
}

bool x::function_decl_ast::get_is_virtual() const
{
	return _is_virtual;
}

void x::function_decl_ast::set_is_virtual( bool val )
{
	_is_virtual = val;
}

const x::stat_ast_ptr & x::function_decl_ast::get_stat() const
{
	return _stat;
}

void x::function_decl_ast::set_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stat = val;
}

std::span<const x::type_ast_ptr> x::function_decl_ast::get_results() const
{
	return _results;
}

void x::function_decl_ast::insert_result( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_results.push_back( val );
}

std::span<const x::parameter_ast_ptr> x::function_decl_ast::get_parameters() const
{
	return _parameters;
}

void x::function_decl_ast::insert_parameter( const x::parameter_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_parameters.push_back( val );
}

x::ast_t x::interface_decl_ast::type() const
{
	return x::ast_t::INTERFACE_DECL;
}

void x::interface_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::interface_decl_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _functions );

	return false;
}

std::span<const x::function_decl_ast_ptr> x::interface_decl_ast::get_functions() const
{
	return _functions;
}

void x::interface_decl_ast::insert_function( const x::function_decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_functions.push_back( val );
}

x::ast_t x::namespace_decl_ast::type() const
{
	return x::ast_t::NAMESPACE_DECL;
}

void x::namespace_decl_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::namespace_decl_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _members );

	return false;
}

std::span<const x::decl_ast_ptr> x::namespace_decl_ast::get_members() const
{
	return _members;
}

void x::namespace_decl_ast::insert_member( const x::decl_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_members.push_back( val );
}

x::ast_t x::empty_stat_ast::type() const
{
	return x::ast_t::EMPTY_STAT;
}

void x::empty_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::extern_stat_ast::type() const
{
	return x::ast_t::EXTERN_STAT;
}

void x::extern_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::call_t x::extern_stat_ast::get_call() const
{
	return _call;
}

void x::extern_stat_ast::set_call( x::call_t val )
{
	_call = val;
}

const std::string & x::extern_stat_ast::get_libname() const
{
	return _libname;
}

void x::extern_stat_ast::set_libname( const std::string & val )
{
	_libname = val;
}

const std::string & x::extern_stat_ast::get_funcname() const
{
	return _funcname;
}

void x::extern_stat_ast::set_funcname( const std::string & val )
{
	_funcname = val;
}

x::ast_t x::compound_stat_ast::type() const
{
	return x::ast_t::COMPOUND_STAT;
}

void x::compound_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::compound_stat_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _stats );

	return false;
}

std::span<const x::stat_ast_ptr> x::compound_stat_ast::get_stats() const
{
	return _stats;
}

void x::compound_stat_ast::insert_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stats.push_back( val );
}

x::ast_t x::await_stat_ast::type() const
{
	return x::ast_t::AWAIT_STAT;
}

void x::await_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::expr_stat_ast_ptr & x::await_stat_ast::get_exp() const
{
	return _exp;
}

void x::await_stat_ast::set_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exp = val;
}

x::ast_t x::yield_stat_ast::type() const
{
	return x::ast_t::YIELD_STAT;
}

void x::yield_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::expr_stat_ast_ptr & x::yield_stat_ast::get_exp() const
{
	return _exp;
}

void x::yield_stat_ast::set_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exp = val;
}

x::ast_t x::if_stat_ast::type() const
{
	return x::ast_t::IF_STAT;
}

void x::if_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::expr_stat_ast_ptr & x::if_stat_ast::get_cond() const
{
	return _cond;
}

void x::if_stat_ast::set_cond( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_cond = val;
}

const x::stat_ast_ptr & x::if_stat_ast::get_then_stat() const
{
	return _then_stat;
}

void x::if_stat_ast::set_then_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_then_stat = val;
}

const x::stat_ast_ptr & x::if_stat_ast::get_else_stat() const
{
	return _else_stat;
}

void x::if_stat_ast::set_else_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_else_stat = val;
}

x::ast_t x::while_stat_ast::type() const
{
	return x::ast_t::WHILE_STAT;
}

void x::while_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::stat_ast_ptr & x::while_stat_ast::get_stat() const
{
	return _stat;
}

void x::while_stat_ast::set_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stat = val;
}

const x::expr_stat_ast_ptr & x::while_stat_ast::get_cond() const
{
	return _cond;
}

void x::while_stat_ast::set_cond( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_cond = val;
}

x::ast_t x::for_stat_ast::type() const
{
	return x::ast_t::FOR_STAT;
}

void x::for_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::stat_ast_ptr & x::for_stat_ast::get_init() const
{
	return _init;
}

void x::for_stat_ast::set_init( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_init = val;
}

const x::expr_stat_ast_ptr & x::for_stat_ast::get_cond() const
{
	return _cond;
}

void x::for_stat_ast::set_cond( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_cond = val;
}

const x::expr_stat_ast_ptr & x::for_stat_ast::get_step() const
{
	return _step;
}

void x::for_stat_ast::set_step( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_step = val;
}

const x::stat_ast_ptr & x::for_stat_ast::get_stat() const
{
	return _stat;
}

void x::for_stat_ast::set_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stat = val;
}

x::ast_t x::foreach_stat_ast::type() const
{
	return x::ast_t::FOREACH_STAT;
}

void x::foreach_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::stat_ast_ptr & x::foreach_stat_ast::get_item() const
{
	return _item;
}

void x::foreach_stat_ast::set_item( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_item = val;
}

const x::expr_stat_ast_ptr & x::foreach_stat_ast::get_collection() const
{
	return _collection;
}

void x::foreach_stat_ast::set_collection( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_collection = val;
}

const x::stat_ast_ptr & x::foreach_stat_ast::get_stat() const
{
	return _stat;
}

void x::foreach_stat_ast::set_stat( const x::stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stat = val;
}

x::ast_t x::switch_stat_ast::type() const
{
	return x::ast_t::SWITCH_STAT;
}

void x::switch_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::switch_stat_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETCC( _cases );

	return false;
}

const x::expr_stat_ast_ptr & x::switch_stat_ast::get_exp() const
{
	return _exp;
}

void x::switch_stat_ast::set_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exp = val;
}

std::span<const x::switch_stat_ast::case_pair> x::switch_stat_ast::get_cases() const
{
	return _cases;
}

void x::switch_stat_ast::insert_case( const x::switch_stat_ast::case_pair & val )
{
	val.first->set_parent( shared_from_this() );
	val.second->set_parent( shared_from_this() );


	_cases.push_back( val );
}

const x::compound_stat_ast_ptr & x::switch_stat_ast::get_defult() const
{
	return _defult;
}

void x::switch_stat_ast::set_defult( const x::compound_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_defult = val;
}

x::ast_t x::break_stat_ast::type() const
{
	return x::ast_t::BREAK_STAT;
}

void x::break_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::return_stat_ast::type() const
{
	return x::ast_t::RETURN_STAT;
}

void x::return_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::return_stat_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _exps );

	return false;
}

std::span<const x::expr_stat_ast_ptr> x::return_stat_ast::get_exps() const
{
	return _exps;
}

void x::return_stat_ast::insert_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exps.push_back( val );
}

x::ast_t x::try_stat_ast::type() const
{
	return x::ast_t::TRY_STAT;
}

void x::try_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::try_stat_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETCC( _catch_stats );

	return false;
}

const x::compound_stat_ast_ptr & x::try_stat_ast::get_try_stat() const
{
	return _try_stat;
}

void x::try_stat_ast::set_try_stat( const x::compound_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_try_stat = val;
}

const x::compound_stat_ast_ptr & x::try_stat_ast::get_final_stat() const
{
	return _final_stat;
}

void x::try_stat_ast::set_final_stat( const x::compound_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_final_stat = val;
}

std::span<const x::try_stat_ast::catch_pair> x::try_stat_ast::get_catch_stats() const
{
	return _catch_stats;
}

void x::try_stat_ast::insert_catch_stat( const x::try_stat_ast::catch_pair & val )
{
	val.first->set_parent( shared_from_this() );
	val.second->set_parent( shared_from_this() );

	_catch_stats.push_back( val );
}

x::ast_t x::throw_stat_ast::type() const
{
	return x::ast_t::THROW_STAT;
}

void x::throw_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::expr_stat_ast_ptr & x::throw_stat_ast::get_exception() const
{
	return _exception;
}

void x::throw_stat_ast::set_exception( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exception = val;
}

x::ast_t x::continue_stat_ast::type() const
{
	return x::ast_t::CONTINUE_STAT;
}

void x::continue_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::local_stat_ast::type() const
{
	return x::ast_t::LOCAL_STAT;
}

void x::local_stat_ast::accept( visitor * val )
{
	val->visit( this );
}

const std::string & x::local_stat_ast::get_name() const
{
	return _name;
}

void x::local_stat_ast::set_name( const std::string & val )
{
	_name = val;
}

bool x::local_stat_ast::get_is_local() const
{
	return _is_local;
}

void x::local_stat_ast::set_is_local( bool val )
{
	_is_local = val;
}

bool x::local_stat_ast::get_is_static() const
{
	return _is_static;
}

void x::local_stat_ast::set_is_static( bool val )
{
	_is_static = val;
}

bool x::local_stat_ast::get_is_thread() const
{
	return _is_thread;
}

void x::local_stat_ast::set_is_thread( bool val )
{
	_is_thread = val;
}

const x::type_ast_ptr & x::local_stat_ast::get_valuetype() const
{
	return _valuetype;
}

void x::local_stat_ast::set_valuetype( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_valuetype = val;
}

const x::initializer_expr_ast_ptr & x::local_stat_ast::get_init() const
{
	return _init;
}

void x::local_stat_ast::set_init( const x::initializer_expr_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_init = val;
}

x::ast_t x::unary_expr_ast::type() const
{
	return x::ast_t::UNARY_EXP;
}

void x::unary_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::operator_t x::unary_expr_ast::get_op() const
{
	return _op;
}

void x::unary_expr_ast::set_op( x::operator_t val )
{
	_op = val;
}

const x::expr_stat_ast_ptr & x::unary_expr_ast::get_exp() const
{
	return _exp;
}

void x::unary_expr_ast::set_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exp = val;
}

x::ast_t x::binary_expr_ast::type() const
{
	return x::ast_t::BINRARY_EXP;
}

void x::binary_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::operator_t x::binary_expr_ast::get_op() const
{
	return _op;
}

void x::binary_expr_ast::set_op( x::operator_t val )
{
	_op = val;
}

const x::expr_stat_ast_ptr & x::binary_expr_ast::get_left() const
{
	return _left;
}

void x::binary_expr_ast::set_left( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_left = val;
}

const x::expr_stat_ast_ptr & x::binary_expr_ast::get_right() const
{
	return _right;
}

void x::binary_expr_ast::set_right( const x::expr_stat_ast_ptr & val )
{
	_right = val;
}

x::ast_t x::bracket_expr_ast::type() const
{
	return x::ast_t::BRACKET_EXP;
}

void x::bracket_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

const x::expr_stat_ast_ptr & x::bracket_expr_ast::get_exp() const
{
	return _exp;
}

void x::bracket_expr_ast::set_exp( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_exp = val;
}

x::ast_t x::closure_expr_ast::type() const
{
	return x::ast_t::CLOSURE_EXP;
}

void x::closure_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::closure_expr_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _results );
	RESETC( _captures );
	RESETC( _parameters );

	return false;
}

const std::string & x::closure_expr_ast::get_name() const
{
	return _name;
}

void x::closure_expr_ast::set_name( const std::string & val )
{
	_name = val;
}

const x::compound_stat_ast_ptr & x::closure_expr_ast::get_stat() const
{
	return _stat;
}

void x::closure_expr_ast::set_stat( const x::compound_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_stat = val;
}

std::span<const x::type_ast_ptr> x::closure_expr_ast::get_results() const
{
	return _results;
}

void x::closure_expr_ast::insert_result( const x::type_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_results.push_back( val );
}

std::span<const x::identifier_expr_ast_ptr> x::closure_expr_ast::get_captures() const
{
	return _captures;
}

void x::closure_expr_ast::insert_capture( const x::identifier_expr_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_captures.push_back( val );
}

std::span<const x::parameter_ast_ptr> x::closure_expr_ast::get_parameters() const
{
	return _parameters;
}

void x::closure_expr_ast::insert_parameter( const x::parameter_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_parameters.push_back( val );
}

x::ast_t x::elements_expr_ast::type() const
{
	return x::ast_t::ELEMENTS_EXP;
}

void x::elements_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::elements_expr_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _elements );

	return false;
}

std::span<const x::expr_stat_ast_ptr> x::elements_expr_ast::get_elements() const
{
	return _elements;
}

void x::elements_expr_ast::insert_element( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_elements.push_back( val );
}

x::ast_t x::arguments_expr_ast::type() const
{
	return x::ast_t::ARGUMENTS_EXP;
}

void x::arguments_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::arguments_expr_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _args );

	return false;
}

std::span<const x::expr_stat_ast_ptr> x::arguments_expr_ast::get_args() const
{
	return _args;
}

void x::arguments_expr_ast::insert_arg( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_args.push_back( val );
}

x::ast_t x::identifier_expr_ast::type() const
{
	return x::ast_t::IDENTIFIER_EXP;
}

void x::identifier_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

const std::string & x::identifier_expr_ast::get_ident() const
{
	return _ident;
}

void x::identifier_expr_ast::set_ident( const std::string & val )
{
	_ident = val;
}

x::ast_t x::initializer_expr_ast::type() const
{
	return x::ast_t::INITIALIZER_EXP;
}

void x::initializer_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::initializer_expr_ast::reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child )
{
	RESETC( _args );

	return false;
}

std::span<const x::expr_stat_ast_ptr> x::initializer_expr_ast::get_args() const
{
	return _args;
}

void x::initializer_expr_ast::insert_arg( const x::expr_stat_ast_ptr & val )
{
	val->set_parent( shared_from_this() );

	_args.push_back( val );
}

x::ast_t x::null_constant_expr_ast::type() const
{
	return x::ast_t::NULL_CONSTANT_EXP;
}

void x::null_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::ast_t x::bool_constant_expr_ast::type() const
{
	return x::ast_t::BOOL_CONSTANT_EXP;
}

void x::bool_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

bool x::bool_constant_expr_ast::get_value() const
{
	return _value;
}

void x::bool_constant_expr_ast::set_value( bool val )
{
	_value = val;
}

x::ast_t x::int8_constant_expr_ast::type() const
{
	return x::ast_t::INT8_CONSTANT_EXP;
}

void x::int8_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::int8 x::int8_constant_expr_ast::get_value() const
{
	return _value;
}

void x::int8_constant_expr_ast::set_value( x::int8 val )
{
	_value = val;
}

x::ast_t x::int16_constant_expr_ast::type() const
{
	return x::ast_t::INT16_CONSTANT_EXP;
}

void x::int16_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::int16 x::int16_constant_expr_ast::get_value() const
{
	return _value;
}

void x::int16_constant_expr_ast::set_value( x::int16 val )
{
	_value = val;
}

x::ast_t x::int32_constant_expr_ast::type() const
{
	return x::ast_t::INT32_CONSTANT_EXP;
}

void x::int32_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::int32 x::int32_constant_expr_ast::get_value() const
{
	return _value;
}

void x::int32_constant_expr_ast::set_value( x::int32 val )
{
	_value = val;
}

x::ast_t x::int64_constant_expr_ast::type() const
{
	return x::ast_t::INT64_CONSTANT_EXP;
}

void x::int64_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::int64 x::int64_constant_expr_ast::get_value() const
{
	return _value;
}

void x::int64_constant_expr_ast::set_value( x::int64 val )
{
	_value = val;
}

x::ast_t x::uint8_constant_expr_ast::type() const
{
	return x::ast_t::UINT8_CONSTANT_EXP;
}

void x::uint8_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::uint8 x::uint8_constant_expr_ast::get_value() const
{
	return _value;
}

void x::uint8_constant_expr_ast::set_value( x::uint8 val )
{
	_value = val;
}

x::ast_t x::uint16_constant_expr_ast::type() const
{
	return x::ast_t::UINT16_CONSTANT_EXP;
}

void x::uint16_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::uint16 x::uint16_constant_expr_ast::get_value() const
{
	return _value;
}

void x::uint16_constant_expr_ast::set_value( x::uint16 val )
{
	_value = val;
}

x::ast_t x::uint32_constant_expr_ast::type() const
{
	return x::ast_t::UINT32_CONSTANT_EXP;
}

void x::uint32_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::uint32 x::uint32_constant_expr_ast::get_value() const
{
	return _value;
}

void x::uint32_constant_expr_ast::set_value( x::uint32 val )
{
	_value = val;
}

x::ast_t x::uint64_constant_expr_ast::type() const
{
	return x::ast_t::UINT64_CONSTANT_EXP;
}

void x::uint64_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::uint64 x::uint64_constant_expr_ast::get_value() const
{
	return _value;
}

void x::uint64_constant_expr_ast::set_value( x::uint64 val )
{
	_value = val;
}

x::ast_t x::float16_constant_expr_ast::type() const
{
	return x::ast_t::FLOAT16_CONSTANT_EXP;
}

void x::float16_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::float16 x::float16_constant_expr_ast::get_value() const
{
	return _value;
}

void x::float16_constant_expr_ast::set_value( x::float16 val )
{
	_value = val;
}

x::ast_t x::float32_constant_expr_ast::type() const
{
	return x::ast_t::FLOAT32_CONSTANT_EXP;
}

void x::float32_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::float32 x::float32_constant_expr_ast::get_value() const
{
	return _value;
}

void x::float32_constant_expr_ast::set_value( x::float32 val )
{
	_value = val;
}

x::ast_t x::float64_constant_expr_ast::type() const
{
	return x::ast_t::FLOAT64_CONSTANT_EXP;
}

void x::float64_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

x::float64 x::float64_constant_expr_ast::get_value() const
{
	return _value;
}

void x::float64_constant_expr_ast::set_value( x::float64 val )
{
	_value = val;
}

x::ast_t x::string_constant_expr_ast::type() const
{
	return x::ast_t::STRING_CONSTANT_EXP;
}

void x::string_constant_expr_ast::accept( visitor * val )
{
	val->visit( this );
}

const std::string & x::string_constant_expr_ast::get_value() const
{
	return _value;
}

void x::string_constant_expr_ast::set_value( const std::string & val )
{
	_value = val;
}
