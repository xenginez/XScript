#pragma once

#include "type.h"

namespace x
{
#define PTR( T ) class T; using T##_ptr = std::shared_ptr< T >;
	PTR( ast );
	PTR( unit_ast );
	PTR( type_ast );
	PTR( import_ast );

	PTR( decl_ast );
	PTR( enum_decl_ast );
	PTR( class_decl_ast );
	PTR( using_decl_ast );
	PTR( enum_element_ast );
	PTR( template_decl_ast );
	PTR( variable_decl_ast );
	PTR( function_decl_ast );
	PTR( parameter_decl_ast );
	PTR( namespace_decl_ast );

	PTR( stat_ast );
	PTR( empty_stat_ast );
	PTR( compound_stat_ast );
	PTR( await_stat_ast );
	PTR( yield_stat_ast );
	PTR( try_stat_ast );
	PTR( catch_stat_ast );
	PTR( throw_stat_ast );
	PTR( if_stat_ast );
	PTR( while_stat_ast );
	PTR( for_stat_ast );
	PTR( foreach_stat_ast );
	PTR( break_stat_ast );
	PTR( return_stat_ast );
	PTR( continue_stat_ast );
	PTR( local_stat_ast );

	PTR( exp_stat_ast );
	PTR( binary_exp_ast );
	PTR( assignment_exp_ast );
	PTR( conditional_exp_ast );
	PTR( logical_or_exp_ast );
	PTR( logical_and_exp_ast );
	PTR( or_exp_ast );
	PTR( xor_exp_ast );
	PTR( and_exp_ast );
	PTR( compare_exp_ast );
	PTR( shift_exp_ast );
	PTR( add_exp_ast );
	PTR( mul_exp_ast );
	PTR( as_exp_ast );
	PTR( is_exp_ast );
	PTR( unary_exp_ast );
	PTR( postfix_exp_ast );
	PTR( index_exp_ast );
	PTR( invoke_exp_ast );
	PTR( member_exp_ast );
	PTR( identifier_exp_ast );
	PTR( closure_exp_ast );
	PTR( arguments_exp_ast );
	PTR( initializers_exp_ast );
	PTR( const_exp_ast );
	PTR( null_const_exp_ast );
	PTR( bool_const_exp_ast );
	PTR( int_const_exp_ast );
	PTR( float_const_exp_ast );
	PTR( string_const_exp_ast );
#undef PTR

	class ast_visitor : public std::enable_shared_from_this<ast_visitor>
	{
	public:
		virtual ~ast_visitor() = default;

	public:
		virtual void visit( x::unit_ast * val );
		virtual void visit( x::type_ast * val );
		virtual void visit( x::import_ast * val );

		virtual void visit( x::enum_decl_ast * val );
		virtual void visit( x::class_decl_ast * val );
		virtual void visit( x::using_decl_ast * val );
		virtual void visit( x::enum_element_ast * val );
		virtual void visit( x::template_decl_ast * val );
		virtual void visit( x::variable_decl_ast * val );
		virtual void visit( x::function_decl_ast * val );
		virtual void visit( x::parameter_decl_ast * val );
		virtual void visit( x::namespace_decl_ast * val );

		virtual void visit( x::empty_stat_ast * val );
		virtual void visit( x::compound_stat_ast * val );
		virtual void visit( x::await_stat_ast * val );
		virtual void visit( x::yield_stat_ast * val );
		virtual void visit( x::try_stat_ast * val );
		virtual void visit( x::catch_stat_ast * val );
		virtual void visit( x::throw_stat_ast * val );
		virtual void visit( x::if_stat_ast * val );
		virtual void visit( x::while_stat_ast * val );
		virtual void visit( x::for_stat_ast * val );
		virtual void visit( x::foreach_stat_ast * val );
		virtual void visit( x::break_stat_ast * val );
		virtual void visit( x::return_stat_ast * val );
		virtual void visit( x::continue_stat_ast * val );
		virtual void visit( x::local_stat_ast * val );

		virtual void visit( x::assignment_exp_ast * val );
		virtual void visit( x::conditional_exp_ast * val );
		virtual void visit( x::logical_or_exp_ast * val );
		virtual void visit( x::logical_and_exp_ast * val );
		virtual void visit( x::or_exp_ast * val );
		virtual void visit( x::xor_exp_ast * val );
		virtual void visit( x::and_exp_ast * val );
		virtual void visit( x::compare_exp_ast * val );
		virtual void visit( x::shift_exp_ast * val );
		virtual void visit( x::add_exp_ast * val );
		virtual void visit( x::mul_exp_ast * val );
		virtual void visit( x::as_exp_ast * val );
		virtual void visit( x::is_exp_ast * val );
		virtual void visit( x::unary_exp_ast * val );
		virtual void visit( x::postfix_exp_ast * val );
		virtual void visit( x::index_exp_ast * val );
		virtual void visit( x::invoke_exp_ast * val );
		virtual void visit( x::member_exp_ast * val );
		virtual void visit( x::identifier_exp_ast * val );
		virtual void visit( x::closure_exp_ast * val );
		virtual void visit( x::arguments_exp_ast * val );
		virtual void visit( x::initializers_exp_ast * val );
		virtual void visit( x::null_const_exp_ast * val );
		virtual void visit( x::bool_const_exp_ast * val );
		virtual void visit( x::int_const_exp_ast * val );
		virtual void visit( x::float_const_exp_ast * val );
		virtual void visit( x::string_const_exp_ast * val );
	};

#define AST( TYPE ) public: x::ast_t ast_type() const override { return x::ast_t::##TYPE; } void accept( ast_visitor * visitor ) override { visitor->visit( static_cast< decltype( this ) >( this ) ); }
	class ast : public std::enable_shared_from_this<ast>
	{
	public:
		virtual ~ast() = default;

	public:
		virtual ast_t ast_type() const = 0;
		virtual void accept( ast_visitor * visitor )
		{
			ASSERT( true, "" );
		}

	public:
		x::source_location location;
	};

	class unit_ast : public ast
	{
		AST( UNIT )

	public:
		std::vector<import_ast_ptr> imports;
		std::vector<namespace_decl_ast_ptr> namespaces;
	};
	class type_ast : public ast
	{
		AST( TYPE )

	public:
		bool is_ref = false;
		bool is_const = false;
		bool is_array = false;
		std::string name;
	};
	class import_ast : public ast
	{
		AST( IMPORT )

	public:
		std::string path;
	};

	class decl_ast : public ast
	{
	public:
		std::string name;
		x::access_t access = x::access_t::PRIVATE;
		x::modify_flag modify = x::modify_flag::NONE;
	};
	class enum_decl_ast : public decl_ast
	{
		AST( ENUM_DECL )

	public:
		std::vector<x::enum_element_ast_ptr> elements;
	};
	class class_decl_ast : public decl_ast
	{
		AST( CLASS_DECL )

	public:
		x::type_ast_ptr base;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class using_decl_ast : public decl_ast
	{
		AST( USING_DECL )

	public:
		x::type_ast_ptr type;
	};
	class enum_element_ast : public ast
	{
		AST( ENUM_DECL )

	public:
		std::string name;
		x::int_const_exp_ast_ptr value;
	};
	class template_decl_ast : public decl_ast
	{
		AST( TEMPLATE_DECL )

	public:
		x::type_ast_ptr base;
		std::vector<template_type> templates;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class variable_decl_ast : public decl_ast
	{
		AST( VARIABLE_DECL )

	public:
		x::type_ast_ptr type;
		x::initializers_exp_ast_ptr init;
	};
	class function_decl_ast : public decl_ast
	{
		AST( FUNCTION_DECL )

	public:
		x::stat_ast_ptr stat;
		x::type_ast_ptr result;
		std::vector<x::parameter_decl_ast_ptr> parameters;
	};
	class parameter_decl_ast : public ast
	{
		AST( PARAMETER_DECL )

	public:
		std::string name;
		x::type_ast_ptr type;
	};
	class namespace_decl_ast : public decl_ast
	{
		AST( NAMESPACE_DECL )

	public:
		std::vector<x::decl_ast_ptr> members;
	};
	
	class stat_ast : public ast
	{
	};
	class empty_stat_ast : public stat_ast
	{
		AST( EMPTY_STAT )
	};
	class compound_stat_ast : public stat_ast
	{
		AST( COMPOUND_STAT )

	public:
		std::vector<x::stat_ast_ptr> stats;
	};
	class await_stat_ast : public stat_ast
	{
		AST( AWAIT_STAT )

	public:
		x::exp_stat_ast_ptr exp;
	};
	class yield_stat_ast : public stat_ast
	{
		AST( YIELD_STAT )

	public:
		bool is_break;
		x::exp_stat_ast_ptr exp;
	};
	class try_stat_ast : public stat_ast
	{
		AST( TRY_STAT )

	public:
		x::compound_stat_ast_ptr body;
		std::vector<x::catch_stat_ast_ptr> catchs;
	};
	class catch_stat_ast : public stat_ast
	{
		AST( CATCH_STAT )

	public:
		x::parameter_decl_ast_ptr type;
		x::compound_stat_ast_ptr body;
	};
	class throw_stat_ast : public stat_ast
	{
		AST( THROW_STAT )

	public:
		x::stat_ast_ptr stat;
	};
	class if_stat_ast : public stat_ast
	{
		AST( IF_STAT )

	public:
		x::exp_stat_ast_ptr exp;
		x::stat_ast_ptr then_stat;
		x::stat_ast_ptr else_stat;
	};
	class while_stat_ast : public stat_ast
	{
		AST( WHILE_STAT )

	public:
		x::stat_ast_ptr stat;
		x::exp_stat_ast_ptr cond;
	};
	class for_stat_ast : public stat_ast
	{
		AST( FOR_STAT )

	public:
		x::stat_ast_ptr init;
		x::exp_stat_ast_ptr cond;
		x::exp_stat_ast_ptr step;
		x::stat_ast_ptr stat;
	};
	class foreach_stat_ast : public stat_ast
	{
		AST( FOREACH_STAT )

	public:
		x::stat_ast_ptr init;
		x::stat_ast_ptr stat;
		x::exp_stat_ast_ptr cond;
	};
	class break_stat_ast : public stat_ast
	{
		AST( BREAK_STAT )
	};
	class return_stat_ast : public stat_ast
	{
		AST( RETURN_STAT )

	public:
		x::exp_stat_ast_ptr exp;
	};
	class continue_stat_ast : public stat_ast
	{
		AST( CONTINUE_STAT )
	};
	class local_stat_ast : public stat_ast
	{
		AST( LOCAL_STAT )

	public:
		std::string name;
		x::modify_flag modify;
		x::type_ast_ptr type;
		x::initializers_exp_ast_ptr init;
	};

	class exp_stat_ast : public stat_ast
	{
	};
	class binary_exp_ast : public exp_stat_ast
	{
	public:
		x::token_t type;
		x::exp_stat_ast_ptr left, right;
	};
	class assignment_exp_ast : public binary_exp_ast
	{
		AST( ASSIGNMENT_EXP )
	};
	class conditional_exp_ast : public exp_stat_ast
	{
		AST( CONDITIONAL_EXP )

	public:
		x::token_t type;
		x::exp_stat_ast_ptr cond, then_exp, else_exp;
	};
	class logical_or_exp_ast : public binary_exp_ast
	{
		AST( LOGICAL_OR_EXP )
	};
	class logical_and_exp_ast : public binary_exp_ast
	{
		AST( LOGICAL_AND_EXP )
	};
	class or_exp_ast : public binary_exp_ast
	{
		AST( OR_EXP )
	};
	class xor_exp_ast : public binary_exp_ast
	{
		AST( XOR_EXP )
	};
	class and_exp_ast : public binary_exp_ast
	{
		AST( AND_EXP )
	};
	class compare_exp_ast : public binary_exp_ast
	{
		AST( COMPARE_EXP )
	};
	class shift_exp_ast : public binary_exp_ast
	{
		AST( SHIFT_EXP )
	};
	class add_exp_ast : public binary_exp_ast
	{
		AST( ADD_EXP )
	};
	class mul_exp_ast : public binary_exp_ast
	{
		AST( MUL_EXP )
	};
	class as_exp_ast : public exp_stat_ast
	{
		AST( AS_EXP )

	public:
		x::exp_stat_ast_ptr value;
		x::type_ast_ptr type;
	};
	class is_exp_ast : public exp_stat_ast
	{
		AST( IS_EXP )

	public:
		x::exp_stat_ast_ptr value;
		x::type_ast_ptr type;
	};
	class unary_exp_ast : public exp_stat_ast
	{
		AST( UNARY_EXP )

	public:
		x::token_t type;
		x::exp_stat_ast_ptr exp;
	};
	class postfix_exp_ast : public unary_exp_ast
	{
		AST( POSTFIX_EXP )
	};
	class index_exp_ast : public exp_stat_ast
	{
		AST( INDEX_EXP )

	public:
		x::exp_stat_ast_ptr left, right;
	};
	class invoke_exp_ast : public exp_stat_ast
	{
		AST( INVOKE_EXP )

	public:
		x::exp_stat_ast_ptr left, right;
	};
	class member_exp_ast : public exp_stat_ast
	{
		AST( MEMBER_EXP )

	public:
		x::exp_stat_ast_ptr left;
		x::identifier_exp_ast_ptr right;
	};
	class identifier_exp_ast : public exp_stat_ast
	{
		AST( IDENTIFIER_EXP )

	public:
		std::string ident;
	};
	class closure_exp_ast : public exp_stat_ast
	{
		AST( CLOSURE_EXP )

	public:
		std::string name;
		x::access_t access = x::access_t::PRIVATE;
		x::modify_flag modify = x::modify_flag::NONE;
		x::stat_ast_ptr stat;
		x::type_ast_ptr result;
		std::vector<x::identifier_exp_ast_ptr> captures;
		std::vector<x::parameter_decl_ast_ptr> parameters;
	};
	class arguments_exp_ast : public exp_stat_ast
	{
		AST( ARGUMENTS_EXP )

	public:
		std::vector<x::exp_stat_ast_ptr> args;
	};
	class initializers_exp_ast : public exp_stat_ast
	{
		AST( INITIALIZERS_EXP )

	public:
		std::vector<x::exp_stat_ast_ptr> args;
	};
	class const_exp_ast : public exp_stat_ast
	{
	};
	class null_const_exp_ast : public const_exp_ast
	{
		AST( NULL_CONST_EXP )
	};
	class bool_const_exp_ast : public const_exp_ast
	{
		AST( BOOL_CONST_EXP )

	public:
		bool value;
	};
	class int_const_exp_ast : public const_exp_ast
	{
		AST( INT_CONST_EXP )

	public:
		int64_t value;
	};
	class float_const_exp_ast : public const_exp_ast
	{
		AST( FLOAT_CONST_EXP )

	public:
		double value;
	};
	class string_const_exp_ast : public const_exp_ast
	{
		AST( STRING_CONST_EXP )

	public:
		std::string value;
	};
#undef AST
}
