#pragma once

#include "type.h"

namespace x
{
	class ast : public std::enable_shared_from_this<ast>
	{
	public:
		virtual ~ast() = default;

	public:
		virtual ast_t ast_type()const = 0;
		virtual void accept( visitor * val ) = 0;

	public:
		x::location location;
	};

	class unit_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<import_ast_ptr> imports;
		std::vector<namespace_decl_ast_ptr> namespaces;
	};
	class import_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string path;
	};
	class attribute_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<std::pair<std::string, std::string>> attributes;
	};

	class type_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		bool is_ref = false;
		bool is_const = false;
		std::string name;
	};
	class func_type_ast : public type_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::type_ast_ptr> parameters;
	};
	class temp_type_ast : public type_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::type_ast_ptr> elements;
	};
	class list_type_ast : public type_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;
	};
	class array_type_ast : public type_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		int layer = 0;
	};

	class enum_element_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		x::expr_stat_ast_ptr value;
	};
	class template_element_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		bool is_multi = false;
	};
	class parameter_element_ast : public ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		x::type_ast_ptr value_type;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::enum_element_ast_ptr> elements;
	};
	class class_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr base;
		x::function_decl_ast_ptr construct;
		x::function_decl_ast_ptr finalize;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class using_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr retype;
	};
	class template_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr base;
		x::compound_stat_ast_ptr where;
		std::vector<x::template_element_ast_ptr> elements;
		x::function_decl_ast_ptr construct;
		x::function_decl_ast_ptr finalize;
		std::vector<x::using_decl_ast_ptr> usings;
		std::vector<x::variable_decl_ast_ptr> variables;
		std::vector<x::function_decl_ast_ptr> functions;
	};
	class variable_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		bool is_local = false;
		bool is_static = false;
		bool is_thread = false;
		x::type_ast_ptr value_type;
		x::initializer_expr_ast_ptr init;
	};
	class function_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		bool is_const = false;
		bool is_async = false;
		bool is_final = false;
		bool is_static = false;
		bool is_virtual = false;
		x::stat_ast_ptr stat;
		std::vector<x::type_ast_ptr> results;
		std::vector<x::parameter_element_ast_ptr> parameters;
	};
	class namespace_decl_ast : public decl_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	};
	class extern_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string libname;
		std::string funcname;
	};
	class compound_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::stat_ast_ptr> stats;
	};
	class await_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr exp;
	};
	class yield_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr exp;
	};
	class new_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::type_ast_ptr newtype;
		x::initializer_expr_ast_ptr init;
	};
	class if_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr stat;
		x::expr_stat_ast_ptr cond;
	};
	class for_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::stat_ast_ptr item;
		x::expr_stat_ast_ptr collection;
		x::stat_ast_ptr stat;
	};
	class switch_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr expr;
		std::vector<std::pair<x::const_expr_ast_ptr, x::compound_stat_ast_ptr>> cases;
		x::compound_stat_ast_ptr defult;
	};
	class break_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	};
	class return_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::expr_stat_ast_ptr> exprs;
	};
	class continue_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	};
	class local_stat_ast : public stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string name;
		bool is_local = false;
		bool is_static = false;
		bool is_thread = false;
		x::type_ast_ptr value_type;
		x::initializer_expr_ast_ptr init;
	};

	class expr_stat_ast : public stat_ast
	{
	};
	class binary_expr_ast : public expr_stat_ast
	{
	public:
		enum class op_t
		{
			NONE,
			ADD,				// +
			SUB,				// - 
			MUL,				// * 
			DIV,				// / 
			MOD,				// %
			AND,				// &
			OR,					// |
			XOR,				// ^
			LEFT_SHIFT,			// <<
			RIGHT_SHIFT,		// >> 
			LAND,				// &&
			LOR,				// ||
			ASSIGN,				// =
			ADD_ASSIGN,			// +=
			SUB_ASSIGN,			// -= 
			MUL_ASSIGN,			// *=
			DIV_ASSIGN,			// /= 
			MOD_ASSIGN,			// %= 
			AND_ASSIGN,			// &= 
			OR_ASSIGN,			// |= 
			XOR_ASSIGN,			// ^= 
			LSHIFT_EQUAL,		// <<=
			RSHIFT_EQUAL,		// >>=
			EQUAL,				// ==
			NOT_EQUAL,			// !=
			LESS,				// <
			LARG,				// >
			LESS_EQUAL,			// <=
			LARG_EQUAL,			// >=
			COMPARE,			// <=>
			AS,					// as
			IS,					// is
			INDEX,				// x[y]
			MEMBER,				// x.y
			INVOKE,				// x(y)
		};

	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		op_t op = op_t::NONE;
		x::expr_stat_ast_ptr left, right;
	};
	class unary_expr_ast : public expr_stat_ast
	{
	public:
		enum class op_t
		{
			NONE,
			PLUS,			// +
			MINUS,			// -
			INC,			// ++i
			DEC,			// --i
			POSTINC,		// i++
			POSTDEC,		// i--
			REV,			// ~
			NOT,			// !
			SIZEOF,			// sizeof
			TYPEOF,			// typeof
		};

	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		op_t op = op_t::NONE;
		x::expr_stat_ast_ptr exp;
	};
	class bracket_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::expr_stat_ast_ptr expr;
	};
	class closure_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		bool is_async = false;
		std::string name;
		x::compound_stat_ast_ptr stat;
		std::vector<x::type_ast_ptr> results;
		std::vector<x::identifier_expr_ast_ptr> captures;
		std::vector<x::parameter_element_ast_ptr> parameters;
	};
	class arguments_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::vector<x::expr_stat_ast_ptr> args;
	};
	class identifier_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string ident;
	};
	class initializer_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;
	};
	class bool_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::int8 value = 0;
	};
	class int16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::int16 value = 0;
	};
	class int32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::int32 value = 0;
	};
	class int64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::int64 value = 0;
	};
	class uint8_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::uint8 value = 0;
	};
	class uint16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::uint16 value = 0;
	};
	class uint32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::uint32 value = 0;
	};
	class uint64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
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
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::float16 value = 0.0f;
	};
	class float32_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::float32 value = 0.0f;
	};
	class float64_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		x::float64 value = 0.0;
	};
	class string_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t ast_type()const override;
		void accept( visitor * val ) override;

	public:
		std::string value;
	};
}
