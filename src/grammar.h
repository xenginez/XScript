#pragma once

#include <istream>

#include "ast.h"

namespace x
{
	class grammar : public std::enable_shared_from_this<grammar>
	{
	public:
		grammar();
		~grammar();

	public:
		x::unit_ast_ptr parse( std::istream & stream, std::string_view name, const std::map<std::string, x::token_t> & tokens = x::token_map );

	private:
		x::unit_ast_ptr unit();
		x::type_ast_ptr type();
		x::import_ast_ptr import();
		x::attribute_ast_ptr attribute();

		x::enum_decl_ast_ptr enum_decl();
		x::class_decl_ast_ptr class_decl();
		x::using_decl_ast_ptr using_decl();
		x::template_decl_ast_ptr template_decl();
		x::variable_decl_ast_ptr variable_decl();
		x::function_decl_ast_ptr function_decl();
		x::parameter_element_ast_ptr parameter_decl();
		x::namespace_decl_ast_ptr namespace_decl();

		x::stat_ast_ptr statement();
		x::extern_stat_ast_ptr extern_stat();
		x::compound_stat_ast_ptr compound_stat();
		x::await_stat_ast_ptr await_stat();
		x::yield_stat_ast_ptr yield_stat();
		x::new_stat_ast_ptr new_stat();
		x::if_stat_ast_ptr if_stat();
		x::while_stat_ast_ptr while_stat();
		x::for_stat_ast_ptr for_stat();
		x::foreach_stat_ast_ptr foreach_stat();
		x::switch_stat_ast_ptr switch_stat();
		x::break_stat_ast_ptr break_stat();
		x::return_stat_ast_ptr return_stat();
		x::continue_stat_ast_ptr continue_stat();
		x::local_stat_ast_ptr local_stat();

		x::expr_stat_ast_ptr express();
		x::expr_stat_ast_ptr assignment_exp();
		x::expr_stat_ast_ptr logical_or_exp();
		x::expr_stat_ast_ptr logical_and_exp();
		x::expr_stat_ast_ptr or_exp();
		x::expr_stat_ast_ptr xor_exp();
		x::expr_stat_ast_ptr and_exp();
		x::expr_stat_ast_ptr compare_exp();
		x::expr_stat_ast_ptr shift_exp();
		x::expr_stat_ast_ptr add_exp();
		x::expr_stat_ast_ptr mul_exp();
		x::expr_stat_ast_ptr as_exp();
		x::expr_stat_ast_ptr is_exp();
		x::expr_stat_ast_ptr sizeof_exp();
		x::expr_stat_ast_ptr typeof_exp();
		x::expr_stat_ast_ptr index_exp();
		x::expr_stat_ast_ptr invoke_exp();
		x::expr_stat_ast_ptr member_exp();
		x::expr_stat_ast_ptr unary_exp();
		x::expr_stat_ast_ptr postfix_exp();
		x::expr_stat_ast_ptr primary_exp();

		x::bracket_expr_ast_ptr bracket_exp();
		x::closure_expr_ast_ptr closure_exp();
		x::arguments_expr_ast_ptr arguments_exp();
		x::identifier_expr_ast_ptr identifier_exp();
		x::initializer_expr_ast_ptr initializer_exp();

		x::const_expr_ast_ptr const_exp();
		x::null_const_expr_ast_ptr null_const_exp();
		x::bool_const_expr_ast_ptr true_const_exp();
		x::bool_const_expr_ast_ptr false_const_exp();
		x::int_const_expr_ast_ptr int_const_exp();
		x::float_const_expr_ast_ptr float_const_exp();
		x::string_const_expr_ast_ptr string_const_exp();

	private:
		x::access_t access();
		std::string type_name();
		x::type_ast_ptr anytype();
		
	private:
		x::token next();
		x::token lookup();
		bool lookup( x::token_t k );
		bool verify( x::token_t k );
		x::token validity( x::token_t k );
		x::token verify( std::initializer_list<x::token_t> list );
		template<typename F> void verify_list( x::token_t beg, x::token_t end, x::token_t step, F && callback )
		{
			validity( beg );
			if ( !verify( end ) )
			{
				do
				{
					if constexpr ( std::is_same_v< std::invoke_result_t<F>, bool > )
					{
						if ( callback() )
							break;
					}
					else if( !lookup( end ) )
					{
						callback();
					}

				} while ( verify( step ) );

				validity( end );
			}
		}

	private:
		char32_t get();
		char32_t peek();
		void push( std::string & str, char32_t c ) const;

	private:
		x::location _location;
		std::istream * _stream = nullptr;
		const std::map<std::string, x::token_t> * _tokenmap = nullptr;
	};
}