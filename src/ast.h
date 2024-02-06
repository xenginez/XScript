#pragma once

#include "type.h"

namespace x
{
#define PTR( T ) class T; using T##_ptr = std::shared_ptr<T>;
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

#define AST public: void accept( ast_visitor * visitor ) override { visitor->visit( static_cast< decltype( this ) >( this ) ); }
	class ast : public std::enable_shared_from_this<ast>
	{
	public:
		virtual ~ast() = default;

	public:
		virtual void accept( ast_visitor * visitor )
		{
			ASSERT( true, "" );
		}

	public:
		x::source_location location;
	};

	class unit_ast : public ast
	{
		AST

	public:
		std::vector<import_ast_ptr> imports;
		std::vector<namespace_decl_ast_ptr> namespaces;
	};
	class type_ast : public ast
	{
		AST

	public:
		bool is_ref = false;
		bool is_const = false;
		bool is_array = false;
		std::string name;
	};
	class import_ast : public ast
	{
	public:
		std::string path;
	};

	class decl_ast : public ast
	{
	public:
		std::string name;
		x::access_t access = x::access_t::PRIVATE;
		x::modify_t modify = x::modify_t::NONE;
	};
	class enum_decl_ast : public decl_ast
	{
		AST

	public:
		std::vector<x::enum_element_ast_ptr> elements;
	};
	class class_decl_ast : public decl_ast
	{
		AST

	public:
		x::type_ast_ptr base;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class using_decl_ast : public decl_ast
	{
		AST

	public:
		x::type_ast_ptr type;
	};
	class enum_element_ast : public ast
	{
		AST

	public:
		std::string name;
		x::int_const_exp_ast_ptr value;
	};
	class template_decl_ast : public decl_ast
	{
		AST

	public:
		x::type_ast_ptr base;
		std::vector<template_type> templates;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class variable_decl_ast : public decl_ast
	{
		AST

	public:
		x::type_ast_ptr type;
		x::initializers_exp_ast_ptr init;
	};
	class function_decl_ast : public decl_ast
	{
		AST

	public:
		x::stat_ast_ptr stat;
		x::type_ast_ptr result;
		std::vector<x::parameter_decl_ast_ptr> parameters;
	};
	class parameter_decl_ast : public ast
	{
		AST

	public:
		std::string name;
		x::type_ast_ptr type;
	};
	class namespace_decl_ast : public decl_ast
	{
		AST

	public:
		std::vector<x::decl_ast_ptr> members;
	};
	
	class stat_ast : public ast
	{
	};
	class empty_stat_ast : public stat_ast
	{
		AST
	};
	class compound_stat_ast : public stat_ast
	{
		AST

	public:
		std::vector<x::stat_ast_ptr> stats;
	};
	class await_stat_ast : public stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr exp;
	};
	class yield_stat_ast : public stat_ast
	{
		AST

	public:
		bool is_break;
		x::exp_stat_ast_ptr exp;
	};
	class try_stat_ast : public stat_ast
	{
		AST

	public:
		x::compound_stat_ast_ptr body;
		std::vector<x::catch_stat_ast_ptr> catchs;
	};
	class catch_stat_ast : public stat_ast
	{
		AST

	public:
		x::parameter_decl_ast_ptr type;
		x::compound_stat_ast_ptr body;
	};
	class throw_stat_ast : public stat_ast
	{
		AST

	public:
		x::stat_ast_ptr stat;
	};
	class if_stat_ast : public stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr exp;
		x::stat_ast_ptr then_stat;
		x::stat_ast_ptr else_stat;
	};
	class while_stat_ast : public stat_ast
	{
		AST

	public:
		x::stat_ast_ptr stat;
		x::exp_stat_ast_ptr cond;
	};
	class for_stat_ast : public stat_ast
	{
		AST

	public:
		x::stat_ast_ptr init;
		x::exp_stat_ast_ptr cond;
		x::exp_stat_ast_ptr step;
		x::stat_ast_ptr stat;
	};
	class foreach_stat_ast : public stat_ast
	{
		AST

	public:
		x::stat_ast_ptr init;
		x::stat_ast_ptr stat;
		x::exp_stat_ast_ptr cond;
	};
	class break_stat_ast : public stat_ast
	{
		AST
	};
	class return_stat_ast : public stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr exp;
	};
	class continue_stat_ast : public stat_ast
	{
		AST
	};
	class local_stat_ast : public stat_ast
	{
		AST

	public:
		std::string name;
		x::modify_t modify;
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
		AST
	};
	class conditional_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::token_t type;
		x::exp_stat_ast_ptr cond, then_exp, else_exp;
	};
	class logical_or_exp_ast : public binary_exp_ast
	{
		AST
	};
	class logical_and_exp_ast : public binary_exp_ast
	{
		AST
	};
	class or_exp_ast : public binary_exp_ast
	{
		AST
	};
	class xor_exp_ast : public binary_exp_ast
	{
		AST
	};
	class and_exp_ast : public binary_exp_ast
	{
		AST
	};
	class compare_exp_ast : public binary_exp_ast
	{
		AST
	};
	class shift_exp_ast : public binary_exp_ast
	{
		AST
	};
	class add_exp_ast : public binary_exp_ast
	{
		AST
	};
	class mul_exp_ast : public binary_exp_ast
	{
		AST
	};
	class as_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr value;
		x::type_ast_ptr type;
	};
	class is_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr value;
		x::type_ast_ptr type;
	};
	class unary_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::token_t type;
		x::exp_stat_ast_ptr exp;
	};
	class postfix_exp_ast : public unary_exp_ast
	{
		AST
	};
	class index_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr left, right;
	};
	class invoke_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr left, right;
	};
	class member_exp_ast : public exp_stat_ast
	{
		AST

	public:
		x::exp_stat_ast_ptr left;
		x::identifier_exp_ast_ptr right;
	};
	class identifier_exp_ast : public exp_stat_ast
	{
		AST

	public:
		std::string ident;
	};
	class closure_exp_ast : public exp_stat_ast
	{
		AST

	public:
		std::string name;
		x::function_decl_ast_ptr function;
		std::vector<x::identifier_exp_ast_ptr> captures;
	};
	class arguments_exp_ast : public exp_stat_ast
	{
		AST

	public:
		std::vector<x::exp_stat_ast_ptr> args;
	};
	class initializers_exp_ast : public exp_stat_ast
	{
		AST

	public:
		std::vector<x::exp_stat_ast_ptr> args;
	};
	class const_exp_ast : public exp_stat_ast
	{
	};
	class null_const_exp_ast : public const_exp_ast
	{
		AST
	};
	class bool_const_exp_ast : public const_exp_ast
	{
		AST

	public:
		bool value;
	};
	class int_const_exp_ast : public const_exp_ast
	{
		AST

	public:
		int64_t value;
	};
	class float_const_exp_ast : public const_exp_ast
	{
		AST

	public:
		double value;
	};
	class string_const_exp_ast : public const_exp_ast
	{
		AST

	public:
		std::string value;
	};
#undef AST
}
