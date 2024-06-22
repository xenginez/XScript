#pragma once

#include "type.h"

namespace x
{
	class ast : public std::enable_shared_from_this<ast>
	{
	public:
		virtual ~ast() = default;

	public:
		virtual ast_t type() const = 0;
		virtual void accept( visitor * val ) = 0;

	public:
		x::location location;
	};

	class unit_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<import_ast_ptr> imports;
		std::vector<namespace_decl_ast_ptr> namespaces;
	};
	class import_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string path;
	};
	class attribute_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::map<std::string, std::string> attributes;
	};

	class type_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool is_ref = false;
		bool is_const = false;
		std::string name;
	};
	class temp_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::type_ast_ptr> elements;
	};
	class func_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::type_ast_ptr> parameters;
	};
	class array_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		int array_layer = 1;
	};

	class decl_ast : public ast
	{
	public:
		std::string name;
		x::access_t access = x::access_t::PRIVATE;
		x::attribute_ast_ptr attr;
	};
	class enum_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::element_decl_ast_ptr> elements;
	};
	class class_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr base;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class using_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr retype;
	};
	class element_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr value;
	};
	class template_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr base;
		x::stat_ast_ptr where;
		std::vector<x::type_ast_ptr> elements;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class variable_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool is_local = false;
		bool is_static = false;
		bool is_thread = false;
		x::type_ast_ptr value_type;
		x::initializers_expr_ast_ptr init;
	};
	class function_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool is_const = false;
		bool is_async = false;
		bool is_static = false;
		bool is_virtual = false;
		x::stat_ast_ptr stat;
		x::type_ast_ptr result;
		std::vector<x::parameter_decl_ast_ptr> parameters;
	};
	class parameter_decl_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		x::type_ast_ptr value_type;
	};
	class namespace_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::decl_ast_ptr> members;
	};
	
	class stat_ast : public ast
	{
	};
	class empty_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class extern_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string libname;
		std::string funcname;
	};
	class compound_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::stat_ast_ptr> stats;
	};
	class await_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr exp;
	};
	class yield_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr exp;
	};
	class new_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr newtype;
		x::initializers_expr_ast_ptr init;
	};
	class try_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::compound_stat_ast_ptr body;
		std::vector<x::catch_stat_ast_ptr> catchs;
	};
	class catch_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::compound_stat_ast_ptr body;
		x::parameter_decl_ast_ptr param;
	};
	class throw_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr stat;
	};
	class if_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr cond;
		x::stat_ast_ptr then_stat;
		x::stat_ast_ptr else_stat;
	};
	class cycle_stat_ast : public stat_ast
	{

	};
	class while_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr stat;
		x::expr_stat_ast_ptr cond;
	};
	class for_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr init;
		x::expr_stat_ast_ptr cond;
		x::expr_stat_ast_ptr step;
		x::stat_ast_ptr stat;
	};
	class foreach_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr item;
		x::expr_stat_ast_ptr collection;
		x::stat_ast_ptr stat;
	};
	class break_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class return_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr exp;
	};
	class continue_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class local_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		bool is_local = false;
		bool is_static = false;
		bool is_thread = false;
		x::type_ast_ptr value_type;
		x::initializers_expr_ast_ptr init;
	};

	class expr_stat_ast : public stat_ast
	{
	};
	class binary_expr_ast : public expr_stat_ast
	{
	public:
		x::token_t token = x::token_t::TK_EOF;
		x::expr_stat_ast_ptr left, right;
	};
	class assignment_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class logical_or_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class logical_and_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class or_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class xor_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class and_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class compare_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class shift_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class add_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class mul_expr_ast : public binary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class as_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr value;
		x::type_ast_ptr cast_type;
	};
	class is_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr value;
		x::type_ast_ptr cast_type;
	};
	class sizeof_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr value;
	};
	class typeof_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr value;
	};
	class unary_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::token_t token = x::token_t::TK_EOF;
		x::expr_stat_ast_ptr exp;
	};
	class postfix_expr_ast : public unary_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class index_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr left, right;
	};
	class invoke_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr left, right;
	};
	class member_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr left;
		x::identifier_expr_ast_ptr right;
	};
	class identifier_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string ident;
	};
	class closure_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool is_async = false;
		std::string name;
		x::type_ast_ptr result;
		x::compound_stat_ast_ptr stat;
		std::vector<x::identifier_expr_ast_ptr> captures;
		std::vector<x::parameter_decl_ast_ptr> parameters;
	};
	class arguments_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::expr_stat_ast_ptr> args;
	};
	class initializers_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::expr_stat_ast_ptr> args;
	};
	class const_expr_ast : public expr_stat_ast
	{
	};
	class null_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	};
	class bool_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool value = false;
	};
	class int_const_expr_ast : public const_expr_ast
	{
	};
	class int8_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int8 value = 0;
	};
	class int16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int16 value = 0;
	};
	class int32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int32 value = 0;
	};
	class int64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int64 value = 0;
	};
	class uint8_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint8 value = 0;
	};
	class uint16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint16 value = 0;
	};
	class uint32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint32 value = 0;
	};
	class uint64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint64 value = 0;
	};
	class float_const_expr_ast : public const_expr_ast
	{
	};
	class float16_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::float16 value = 0.0f;
	};
	class float32_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::float32 value = 0.0f;
	};
	class float64_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::float64 value = 0.0;
	};
	class string_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::string value;
	};
}
