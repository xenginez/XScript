#pragma once

#include <istream>

#include "ast.h"

namespace x
{
	class grammar
	{
	public:
		grammar( std::istream & stream, std::string_view name, const std::map<std::string, x::token_t> tokens = x::token_map );
		~grammar();

	public:
		x::unit_ast_ptr unit();
		x::type_ast_ptr type();
		x::import_ast_ptr import();
		x::attribute_ast_ptr attribute();

		x::enum_decl_ast_ptr enum_decl();
		x::flag_decl_ast_ptr flag_decl();
		x::class_decl_ast_ptr class_decl();
		x::using_decl_ast_ptr using_decl();
		x::template_decl_ast_ptr template_decl();
		x::variable_decl_ast_ptr variable_decl();
		x::function_decl_ast_ptr function_decl();
		x::parameter_decl_ast_ptr parameter_decl();
		x::namespace_decl_ast_ptr namespace_decl();

		x::stat_ast_ptr stat();
		x::extern_stat_ast_ptr extern_stat();
		x::compound_stat_ast_ptr compound_stat();
		x::await_stat_ast_ptr await_stat();
		x::yield_stat_ast_ptr yield_stat();
		x::try_stat_ast_ptr try_stat();
		x::catch_stat_ast_ptr catch_stat();
		x::throw_stat_ast_ptr throw_stat();
		x::if_stat_ast_ptr if_stat();
		x::while_stat_ast_ptr while_stat();
		x::for_stat_ast_ptr for_stat();
		x::foreach_stat_ast_ptr foreach_stat();
		x::break_stat_ast_ptr break_stat();
		x::return_stat_ast_ptr return_stat();
		x::continue_stat_ast_ptr continue_stat();
		x::local_stat_ast_ptr local_stat();

		x::exp_stat_ast_ptr exp_stat();
		x::exp_stat_ast_ptr assignment_exp();
		x::exp_stat_ast_ptr logical_or_exp();
		x::exp_stat_ast_ptr logical_and_exp();
		x::exp_stat_ast_ptr or_exp();
		x::exp_stat_ast_ptr xor_exp();
		x::exp_stat_ast_ptr and_exp();
		x::exp_stat_ast_ptr compare_exp();
		x::exp_stat_ast_ptr shift_exp();
		x::exp_stat_ast_ptr add_exp();
		x::exp_stat_ast_ptr mul_exp();
		x::exp_stat_ast_ptr as_exp();
		x::exp_stat_ast_ptr is_exp();
		x::exp_stat_ast_ptr sizeof_exp();
		x::exp_stat_ast_ptr typeof_exp();
		x::exp_stat_ast_ptr unary_exp();
		x::exp_stat_ast_ptr postfix_exp();
		x::exp_stat_ast_ptr index_exp();
		x::exp_stat_ast_ptr invoke_exp();
		x::exp_stat_ast_ptr member_exp();
		x::exp_stat_ast_ptr primary_exp();
		x::closure_exp_ast_ptr closure_exp();
		x::arguments_exp_ast_ptr arguments_exp();
		x::identifier_exp_ast_ptr identifier_exp();
		x::initializers_exp_ast_ptr initializers_exp();
		x::const_exp_ast_ptr const_exp();
		x::null_const_exp_ast_ptr null_const_exp();
		x::bool_const_exp_ast_ptr true_const_exp();
		x::bool_const_exp_ast_ptr false_const_exp();
		x::int_const_exp_ast_ptr int_const_exp();
		x::float_const_exp_ast_ptr float_const_exp();
		x::string_const_exp_ast_ptr string_const_exp();

	private:
		x::access_t access();
		std::string type_name();
		x::type_ast_ptr type( std::string_view name, bool is_ref = false, bool is_const = false, int array = 0 );
		std::string location_to_name( const x::location & location, std::string_view suffix = "" );

	private:
		x::token next();
		x::token lookup();
		bool verify( x::token_t k );
		x::token_t verify( std::initializer_list<x::token_t> list );
		x::token validity( x::token_t k );

	private:
		int get();
		int peek();
		void push( std::string & str, int c ) const;

	private:
		std::istream _stream;
		x::location _location;
		std::map<std::string, x::token_t> _tokenmap;
	};
}