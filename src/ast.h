#pragma once

#include <span>

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
		virtual void reset_child( const x::ast_ptr & old_child, const x::ast_ptr & new_child );

	public:
		x::ast_ptr get_parent() const;
		void set_parent( const x::ast_ptr & val );
		const x::location & get_location() const;
		void set_location( const x::location & val );

	private:
		x::ast_wptr _parent;
		x::location _location;
	};

	class unit_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::import_ast_ptr> get_imports() const;
		void insert_import( const x::import_ast_ptr & val );
		std::span<const x::namespace_decl_ast_ptr> get_namespaces() const;
		void insert_namespace( const x::namespace_decl_ast_ptr & val );

	private:
		std::vector<x::import_ast_ptr> _imports;
		std::vector<x::namespace_decl_ast_ptr> _namespaces;
	};
	class import_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_path() const;
		void set_path( const std::string & val );

	private:
		std::string _path;
	};
	class attribute_ast : public ast
	{
	public:
		using string_pair = std::pair<std::string, std::string>;

	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const string_pair> get_attributes() const;
		void insert_attribute( const string_pair & val );

	private:
		std::vector<string_pair> _attributes;
	};

	class type_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool get_is_ref() const;
		void set_is_ref( bool val );
		bool get_is_const() const;
		void set_is_const( bool val );
		const std::string & get_name() const;
		void set_name( const std::string & val );

	private:
		bool _is_ref = false;
		bool _is_const = false;
		std::string _name;
	};
	class func_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::type_ast_ptr> get_parameters() const;
		void insert_parameter( const x::type_ast_ptr & val );

	private:
		std::vector<x::type_ast_ptr> _parameters;
	};
	class temp_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::type_ast_ptr> get_elements() const;
		void insert_element( const x::type_ast_ptr & val );

	private:
		std::vector<x::type_ast_ptr> _elements;
	};
	class list_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;
	};
	class array_type_ast : public type_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		int get_layer() const;
		void set_layer( int val );

	private:
		int _layer = 0;
	};

	class enum_element_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_name() const;
		void set_name( const std::string & val );
		const x::expr_stat_ast_ptr & get_value() const;
		void set_value( const x::expr_stat_ast_ptr & val );

	private:
		std::string _name;
		x::expr_stat_ast_ptr _value;
	};
	class template_element_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_name() const;
		void set_name( const std::string & val );
		bool get_is_multi() const;
		void set_is_multi( bool val );

	private:
		std::string _name;
		bool _is_multi = false;
	};
	class parameter_element_ast : public ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_name() const;
		void set_name( const std::string & val );
		const x::type_ast_ptr & get_valuetype() const;
		void set_valuetype( const x::type_ast_ptr & val );

	private:
		std::string _name;
		x::type_ast_ptr _valuetype;
	};

	class decl_ast : public ast
	{
	public:
		const std::string & get_name() const;
		void set_name( const std::string & val );
		x::access_t get_access() const;
		void set_access( x::access_t val );
		const x::attribute_ast_ptr & get_attr() const;
		void set_attr( const x::attribute_ast_ptr & val );

	private:
		std::string _name;
		x::access_t _access = x::access_t::PRIVATE;
		x::attribute_ast_ptr _attr;
	};
	class enum_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::enum_element_ast_ptr> get_elements() const;
		void insert_element( const x::enum_element_ast_ptr & val );

	private:
		std::vector<x::enum_element_ast_ptr> _elements;
	};
	class using_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::type_ast_ptr & get_retype() const;
		void set_retype( const x::type_ast_ptr & val );

	private:
		x::type_ast_ptr _retype;
	};
	class class_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::type_ast_ptr & get_base() const;
		void set_base( const x::type_ast_ptr & val );
		const x::function_decl_ast_ptr & get_construct() const;
		void set_construct( const x::function_decl_ast_ptr & val );
		const x::function_decl_ast_ptr & get_finalize() const;
		void set_finalize( const x::function_decl_ast_ptr & val );
		std::span<const x::type_ast_ptr> get_interfaces() const;
		void insert_interface( const x::type_ast_ptr & val );
		std::span<const x::using_decl_ast_ptr> get_usings() const;
		void insert_using( const x::using_decl_ast_ptr & val );
		std::span<const x::variable_decl_ast_ptr> get_variables() const;
		void insert_variable( const x::variable_decl_ast_ptr & val );
		std::span<const x::function_decl_ast_ptr> get_functions() const;
		void insert_function( const x::function_decl_ast_ptr & val );

	private:
		x::type_ast_ptr _base;
		x::function_decl_ast_ptr _construct;
		x::function_decl_ast_ptr _finalize;
		std::vector<x::type_ast_ptr> _interfaces;
		std::vector<x::using_decl_ast_ptr> _usings;
		std::vector<x::variable_decl_ast_ptr> _variables;
		std::vector<x::function_decl_ast_ptr> _functions;
	};
	class template_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::type_ast_ptr & get_base() const;
		void set_base( const x::type_ast_ptr & val );
		const x::compound_stat_ast_ptr & get_where() const;
		void set_where( const x::compound_stat_ast_ptr & val );
		const x::function_decl_ast_ptr & get_construct() const;
		void set_construct( const x::function_decl_ast_ptr & val );
		const x::function_decl_ast_ptr & get_finalize() const;
		void set_finalize( const x::function_decl_ast_ptr & val );
		std::span<const x::type_ast_ptr> get_interfaces() const;
		void insert_interface( const x::type_ast_ptr & val );
		std::span<const x::using_decl_ast_ptr> get_usings() const;
		void insert_using( const x::using_decl_ast_ptr & val );
		std::span<const x::variable_decl_ast_ptr> get_variables() const;
		void insert_variable( const x::variable_decl_ast_ptr & val );
		std::span<const x::function_decl_ast_ptr> get_functions() const;
		void insert_function( const x::function_decl_ast_ptr & val );
		std::span<const x::template_element_ast_ptr> get_elements() const;
		void insert_element( const x::template_element_ast_ptr & val );

	private:
		x::type_ast_ptr _base;
		x::compound_stat_ast_ptr _where;
		x::function_decl_ast_ptr _construct;
		x::function_decl_ast_ptr _finalize;
		std::vector<x::type_ast_ptr> _interfaces;
		std::vector<x::using_decl_ast_ptr> _usings;
		std::vector<x::variable_decl_ast_ptr> _variables;
		std::vector<x::function_decl_ast_ptr> _functions;
		std::vector<x::template_element_ast_ptr> _elements;
	};
	class variable_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool get_is_local() const;
		void set_is_local( bool val );
		bool get_is_static() const;
		void set_is_static( bool val );
		bool get_is_thread() const;
		void set_is_thread( bool val );
		const x::type_ast_ptr & get_valuetype() const;
		void set_valuetype( const x::type_ast_ptr & val );
		const x::initializer_expr_ast_ptr & get_init() const;
		void set_init( const x::initializer_expr_ast_ptr & val );

	private:
		bool _is_local = false;
		bool _is_static = false;
		bool _is_thread = false;
		x::type_ast_ptr _valuetype;
		x::initializer_expr_ast_ptr _init;
	};
	class function_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool get_is_const() const;
		void set_is_const( bool val );
		bool get_is_async() const;
		void set_is_async( bool val );
		bool get_is_final() const;
		void set_is_final( bool val );
		bool get_is_static() const;
		void set_is_static( bool val );
		bool get_is_virtual() const;
		void set_is_virtual( bool val );
		const x::stat_ast_ptr & get_stat() const;
		void set_stat( const x::stat_ast_ptr & val );
		std::span<const x::type_ast_ptr> get_results() const;
		void insert_result( const x::type_ast_ptr & val );
		std::span<const x::parameter_element_ast_ptr> get_parameters() const;
		void insert_parameter( const x::parameter_element_ast_ptr & val );

	private:
		bool _is_const = false;
		bool _is_async = false;
		bool _is_final = false;
		bool _is_static = false;
		bool _is_virtual = false;
		x::stat_ast_ptr _stat;
		std::vector<x::type_ast_ptr> _results;
		std::vector<x::parameter_element_ast_ptr> _parameters;
	};
	class interface_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::function_decl_ast_ptr> get_functions() const;
		void insert_function( const x::function_decl_ast_ptr & val );

	private:
		std::vector<x::function_decl_ast_ptr> _functions;
	};
	class namespace_decl_ast : public decl_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::decl_ast_ptr> get_members() const;
		void insert_member( const x::decl_ast_ptr & val );

	private:
		std::vector<x::decl_ast_ptr> _members;
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
		const std::string & get_libname() const;
		void set_libname( const std::string & val );
		const std::string & get_funcname() const;
		void set_funcname( const std::string & val );

	private:
		std::string _libname;
		std::string _funcname;
	};
	class compound_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::stat_ast_ptr> get_stats() const;
		void insert_stat( const x::stat_ast_ptr & val );

	private:
		std::vector<x::stat_ast_ptr> _stats;
	};
	class await_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::expr_stat_ast_ptr & get_exp() const;
		void set_exp( const x::expr_stat_ast_ptr & val );

	private:
		x::expr_stat_ast_ptr _exp;
	};
	class yield_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::expr_stat_ast_ptr & get_exp() const;
		void set_exp( const x::expr_stat_ast_ptr & val );

	private:
		x::expr_stat_ast_ptr _exp;
	};
	class new_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::type_ast_ptr & get_newtype() const;
		void set_newtype( const x::type_ast_ptr & val );
		const x::initializer_expr_ast_ptr & get_init() const;
		void set_init( const x::initializer_expr_ast_ptr & val );

	private:
		x::type_ast_ptr _newtype;
		x::initializer_expr_ast_ptr _init;
	};
	class if_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::expr_stat_ast_ptr & get_cond() const;
		void set_cond( const x::expr_stat_ast_ptr & val );
		const x::stat_ast_ptr & get_then_stat() const;
		void set_then_stat( const x::stat_ast_ptr & val );
		const x::stat_ast_ptr & get_else_stat() const;
		void set_else_stat( const x::stat_ast_ptr & val );

	private:
		x::expr_stat_ast_ptr _cond;
		x::stat_ast_ptr _then_stat;
		x::stat_ast_ptr _else_stat;
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
		const x::stat_ast_ptr & get_stat() const;
		void set_stat( const x::stat_ast_ptr & val );
		const x::expr_stat_ast_ptr & get_cond() const;
		void set_cond( const x::expr_stat_ast_ptr & val );

	private:
		x::stat_ast_ptr _stat;
		x::expr_stat_ast_ptr _cond;
	};
	class for_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::stat_ast_ptr & get_init() const;
		void set_init( const x::stat_ast_ptr & val );
		const x::expr_stat_ast_ptr & get_cond() const;
		void set_cond( const x::expr_stat_ast_ptr & val );
		const x::expr_stat_ast_ptr & get_step() const;
		void set_step( const x::expr_stat_ast_ptr & val );
		const x::stat_ast_ptr & get_stat() const;
		void set_stat( const x::stat_ast_ptr & val );

	private:
		x::stat_ast_ptr _init;
		x::expr_stat_ast_ptr _cond;
		x::expr_stat_ast_ptr _step;
		x::stat_ast_ptr _stat;
	};
	class foreach_stat_ast : public cycle_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::stat_ast_ptr & get_item() const;
		void set_item( const x::stat_ast_ptr & val );
		const x::expr_stat_ast_ptr & get_collection() const;
		void set_collection( const x::expr_stat_ast_ptr & val );
		const x::stat_ast_ptr & get_stat() const;
		void set_stat( const x::stat_ast_ptr & val );

	private:
		x::stat_ast_ptr _item;
		x::expr_stat_ast_ptr _collection;
		x::stat_ast_ptr _stat;
	};
	class switch_stat_ast : public stat_ast
	{
	public:
		using case_pair = std::pair<x::const_expr_ast_ptr, x::compound_stat_ast_ptr>;

	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::expr_stat_ast_ptr & get_exp() const;
		void set_exp( const x::expr_stat_ast_ptr & val );
		std::span<const case_pair> get_cases() const;
		void insert_case( const case_pair & val );
		const x::compound_stat_ast_ptr & get_defult() const;
		void set_defult( const x::compound_stat_ast_ptr & val );

	private:
		x::expr_stat_ast_ptr _exp;
		std::vector<case_pair> _cases;
		x::compound_stat_ast_ptr _defult;
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
		std::span<const x::expr_stat_ast_ptr> get_exps() const;
		void insert_exp( const x::expr_stat_ast_ptr & val );

	private:
		std::vector<x::expr_stat_ast_ptr> _exps;
	};
	class try_stat_ast : public stat_ast
	{
	public:
		using catch_pair = std::pair<x::parameter_element_ast_ptr, x::compound_stat_ast_ptr>;

	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::compound_stat_ast_ptr & get_try_stat() const;
		void set_try_stat( const x::compound_stat_ast_ptr & val );
		const x::compound_stat_ast_ptr & get_final_stat() const;
		void set_final_stat( const x::compound_stat_ast_ptr & val );
		std::span<const catch_pair> get_catch_stats() const;
		void insert_catch_stat( const catch_pair & val );

	private:
		x::compound_stat_ast_ptr _try_stat;
		x::compound_stat_ast_ptr _final_stat;
		std::vector<catch_pair> _catch_stats;
	};
	class throw_stat_ast : public stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::new_stat_ast_ptr & get_exception() const;
		void set_exception( const x::new_stat_ast_ptr & val );

	private:
		x::new_stat_ast_ptr _exception;
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
		const std::string & get_name() const;
		void set_name( const std::string & val );
		bool get_is_local() const;
		void set_is_local( bool val );
		bool get_is_static() const;
		void set_is_static( bool val );
		bool get_is_thread() const;
		void set_is_thread( bool val );
		const x::type_ast_ptr & get_valuetype() const;
		void set_valuetype( const x::type_ast_ptr & val );
		const x::initializer_expr_ast_ptr & get_init() const;
		void set_init( const x::initializer_expr_ast_ptr & val );

	private:
		std::string _name;
		bool _is_local = false;
		bool _is_static = false;
		bool _is_thread = false;
		x::type_ast_ptr _valuetype;
		x::initializer_expr_ast_ptr _init;
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
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		op_t get_op() const;
		void set_op( op_t val );
		const x::expr_stat_ast_ptr & get_left() const;
		void set_left( const x::expr_stat_ast_ptr & val );
		const x::expr_stat_ast_ptr & get_right() const;
		void set_right( const x::expr_stat_ast_ptr & val );

	private:
		op_t _op = op_t::NONE;
		x::expr_stat_ast_ptr _left, _right;
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
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		op_t get_op() const;
		void set_op( op_t val );
		const x::expr_stat_ast_ptr & get_exp() const;
		void set_exp( const x::expr_stat_ast_ptr & val );

	private:
		op_t _op = op_t::NONE;
		x::expr_stat_ast_ptr _exp;
	};
	class bracket_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const x::expr_stat_ast_ptr & get_exp() const;
		void set_exp( const x::expr_stat_ast_ptr & val );

	private:
		x::expr_stat_ast_ptr _exp;
	};
	class closure_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		bool get_is_async() const;
		void set_is_async( bool val );
		const std::string & get_name() const;
		void set_name( const std::string & val );
		const x::compound_stat_ast_ptr & get_stat() const;
		void set_stat( const x::compound_stat_ast_ptr & val );
		std::span<const x::type_ast_ptr> get_results() const;
		void insert_result( const x::type_ast_ptr & val );
		std::span<const x::identifier_expr_ast_ptr> get_captures() const;
		void insert_capture( const x::identifier_expr_ast_ptr & val );
		std::span<const x::parameter_element_ast_ptr> get_parameters() const;
		void insert_parameter( const x::parameter_element_ast_ptr & val );

	private:
		bool _is_async = false;
		std::string _name;
		x::compound_stat_ast_ptr _stat;
		std::vector<x::type_ast_ptr> _results;
		std::vector<x::identifier_expr_ast_ptr> _captures;
		std::vector<x::parameter_element_ast_ptr> _parameters;
	};
	class arguments_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::expr_stat_ast_ptr> get_args() const;
		void insert_arg( const x::expr_stat_ast_ptr & val );

	private:
		std::vector<x::expr_stat_ast_ptr> _args;
	};
	class identifier_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_ident() const;
		void set_ident( const std::string & val );

	private:
		std::string _ident;
	};
	class initializer_expr_ast : public expr_stat_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		std::span<const x::expr_stat_ast_ptr> get_args() const;
		void insert_arg( const x::expr_stat_ast_ptr & val );

	private:
		std::vector<x::expr_stat_ast_ptr> _args;
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
		bool get_value() const;
		void set_value( bool val );

	private:
		bool _value = false;
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
		x::int8 get_value() const;
		void set_value( x::int8 val );

	private:
		x::int8 _value = 0;
	};
	class int16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int16 get_value() const;
		void set_value( x::int16 val );

	private:
		x::int16 _value = 0;
	};
	class int32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int32 get_value() const;
		void set_value( x::int32 val );

	private:
		x::int32 _value = 0;
	};
	class int64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::int64 get_value() const;
		void set_value( x::int64 val );

	private:
		x::int64 _value = 0;
	};
	class uint8_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint8 get_value() const;
		void set_value( x::uint8 val );

	private:
		x::uint8 _value = 0;
	};
	class uint16_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint16 get_value() const;
		void set_value( x::uint16 val );

	private:
		x::uint16 _value = 0;
	};
	class uint32_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint32 get_value() const;
		void set_value( x::uint32 val );

	private:
		x::uint32 _value = 0;
	};
	class uint64_const_expr_ast : public int_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::uint64 get_value() const;
		void set_value( x::uint64 val );

	private:
		x::uint64 _value = 0;
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
		x::float16 get_value() const;
		void set_value( x::float16 val );

	private:
		x::float16 _value = 0.0f;
	};
	class float32_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::float32 get_value() const;
		void set_value( x::float32 val );

	private:
		x::float32 _value = 0.0f;
	};
	class float64_const_expr_ast : public float_const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		x::float64 get_value() const;
		void set_value( x::float64 val );

	private:
		x::float64 _value = 0.0;
	};
	class string_const_expr_ast : public const_expr_ast
	{
	public:
		x::ast_t type() const override;
		void accept( visitor * val ) override;

	public:
		const std::string & get_value() const;
		void set_value( const std::string & val );

	private:
		std::string _value;
	};
}
