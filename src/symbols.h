#pragma once

#include <deque>
#include <variant>

#include "type.h"

namespace x
{
	class symbol
	{
	public:
		symbol() = default;
		virtual ~symbol() = default;

	private:
		symbol( symbol && ) = delete;
		symbol( const symbol & ) = delete;
		symbol & operator=( symbol && ) = delete;
		symbol & operator=( const symbol & ) = delete;

	public:
		bool is_ast() const;
		bool is_type() const;
		bool is_scope() const;
		bool is_unit() const;
		bool is_enum() const;
		bool is_alias() const;
		bool is_class() const;
		bool is_block() const;
		bool is_cycle() const;
		bool is_local() const;
		bool is_param() const;
		bool is_element() const;
		bool is_function() const;
		bool is_variable() const;
		bool is_template() const;
		bool is_namespace() const;
		bool is_foundation() const;
		bool is_nativefunc() const;
		bool is_builtinfunc() const;

	public:
		x::ast_symbol * cast_ast();
		x::type_symbol * cast_type();
		x::scope_symbol * cast_scope();
		x::unit_symbol * cast_unit();
		x::enum_symbol * cast_enum();
		x::alias_symbol * cast_alias();
		x::class_symbol * cast_class();
		x::block_symbol * cast_block();
		x::cycle_symbol * cast_cycle();
		x::local_symbol * cast_local();
		x::param_symbol * cast_param();
		x::element_symbol * cast_element();
		x::function_symbol * cast_function();
		x::variable_symbol * cast_variable();
		x::template_symbol * cast_template();
		x::namespace_symbol * cast_namespace();
		x::foundation_symbol * cast_foundation();
		x::nativefunc_symbol * cast_nativefunc();
		x::builtinfunc_symbol * cast_builtinfunc();

	public:
		x::symbol_t type = x::symbol_t::UNIT;
		std::string name;
		std::string fullname;
		x::symbol * parent = nullptr;
	};
	class ast_symbol
	{
	public:
		x::symbol * cast_symbol();

	public:
		virtual x::ast_ptr ast() const = 0;
	};
	class type_symbol
	{
	public:
		x::symbol * cast_symbol();

	public:
		virtual x::uint64 size() const = 0;
	};
	class scope_symbol
	{
	public:
		x::symbol * cast_symbol();

	public:
		virtual void add_child( x::symbol * val ) = 0;
		virtual x::symbol * find_child( std::string_view name ) const = 0;
	};

	class unit_symbol : public x::symbol, public x::ast_symbol, public x::scope_symbol
	{
	public:
		unit_symbol();
		~unit_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::unit_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::unit_ast_ptr unit_ast;
		std::vector<x::symbol *> children;
	};
	class enum_symbol : public x::symbol, public x::ast_symbol, public x::type_symbol, public x::scope_symbol
	{
	public:
		enum_symbol();
		~enum_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::enum_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::enum_decl_ast_ptr enum_ast;
		std::vector<x::element_symbol *> elements;
	};
	class alias_symbol : public x::symbol, public x::ast_symbol, public x::type_symbol
	{
	public:
		alias_symbol();
		~alias_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::using_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;

	public:
		x::type_symbol * retype = nullptr;
		x::using_decl_ast_ptr using_ast;
	};
	class class_symbol : public x::symbol, public x::ast_symbol, public x::type_symbol, public x::scope_symbol
	{
	public:
		class_symbol();
		~class_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::class_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		x::class_decl_ast_ptr class_ast;
		std::vector<x::alias_symbol *> aliases;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class block_symbol : public x::symbol, public x::ast_symbol, public x::scope_symbol
	{
	public:
		block_symbol();
		~block_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::compound_stat_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::compound_stat_ast_ptr compound_ast;
		std::vector<x::local_symbol *> locals;
		std::vector<x::block_symbol *> blocks;
	};
	class cycle_symbol : public x::symbol, public x::ast_symbol, public x::scope_symbol
	{
	public:
		cycle_symbol();
		~cycle_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::cycle_stat_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::cycle_stat_ast_ptr cycle_ast;
		std::vector<x::local_symbol *> locals;
		std::vector<x::block_symbol *> blocks;
	};
	class local_symbol : public x::symbol, public x::ast_symbol
	{
	public:
		local_symbol();
		~local_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::local_stat_ast_ptr cast_ast() const;

	public:
		x::local_stat_ast_ptr local_ast;
		x::type_symbol * valuetype = nullptr;
	};
	class param_symbol : public x::symbol, public x::ast_symbol
	{
	public:
		param_symbol();
		~param_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::parameter_decl_ast_ptr cast_ast() const;

	public:
		x::type_symbol * valuetype = nullptr;
		x::parameter_decl_ast_ptr parameter_ast;
	};
	class element_symbol : public x::symbol, public x::ast_symbol
	{
	public:
		element_symbol();
		~element_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::element_decl_ast_ptr cast_ast() const;

	public:
		x::element_decl_ast_ptr element_ast;
	};
	class variable_symbol : public x::symbol, public x::ast_symbol
	{
	public:
		variable_symbol();
		~variable_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::variable_decl_ast_ptr cast_ast() const;

	public:
		x::type_symbol * value = nullptr;
		x::variable_decl_ast_ptr variable_ast;
	};
	class function_symbol : public x::symbol, public x::ast_symbol, public x::scope_symbol
	{
	public:
		function_symbol();
		~function_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::function_decl_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::type_symbol * result = nullptr;
		x::block_symbol * block = nullptr;
		x::function_decl_ast_ptr function_ast;
		std::vector<x::param_symbol *> parameters;
	};
	class template_symbol : public x::symbol, public x::ast_symbol, public x::type_symbol, public x::scope_symbol
	{
	public:
		template_symbol();
		~template_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::template_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		x::template_decl_ast_ptr template_ast;
		std::vector<x::alias_symbol *> aliases;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class namespace_symbol : public x::symbol, public x::ast_symbol, public x::scope_symbol
	{
	public:
		namespace_symbol();
		~namespace_symbol() override;

	public:
		x::ast_ptr ast() const override;
		x::namespace_decl_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::type_symbol *> children;
		x::namespace_decl_ast_ptr namespace_ast;
	};
	class foundation_symbol : public x::symbol, public x::type_symbol, public x::scope_symbol
	{
	public:
		foundation_symbol();
		~foundation_symbol() override;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::uint64 sz = 0;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class nativefunc_symbol : public x::symbol, public x::scope_symbol
	{
	public:
		nativefunc_symbol();
		~nativefunc_symbol() override;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		bool is_await = false;
		void * callback = nullptr;
		x::type_symbol * result = nullptr;
		std::vector<x::param_symbol *> parameters;
	};
	class builtinfunc_symbol : public x::symbol, public x::scope_symbol
	{
	public:
		builtinfunc_symbol();
		~builtinfunc_symbol() override;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::ast_ptr transferred( const x::symbols_ptr & symbols, const x::ast_ptr & val ) const;

	public:
		x::builtin_ptr built = nullptr;
		x::type_symbol * result = nullptr;
		std::vector<x::param_symbol *> parameters;
	};

	using symbol_variant = std::variant<
		std::monostate,
		unit_symbol,
		enum_symbol,
		alias_symbol,
		class_symbol,
		block_symbol,
		cycle_symbol,
		local_symbol,
		param_symbol,
		element_symbol,
		function_symbol,
		variable_symbol,
		template_symbol,
		namespace_symbol,
		foundation_symbol,
		nativefunc_symbol,
		builtinfunc_symbol>;

	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void add_symbol( x::symbol * val );
		x::unit_symbol * add_unit( x::unit_ast * val );
		x::enum_symbol * add_enum( x::enum_decl_ast * val );
		x::alias_symbol * add_alias( x::using_decl_ast * val );
		x::class_symbol * add_class( x::class_decl_ast * val );
		x::block_symbol * add_block( x::compound_stat_ast * val );
		x::cycle_symbol * add_cycle( x::cycle_stat_ast * val );
		x::local_symbol * add_local( x::local_stat_ast * val );
		x::param_symbol * add_param( x::parameter_decl_ast * val );
		x::element_symbol * add_element( x::element_decl_ast * val );
		x::function_symbol * add_function( x::function_decl_ast * val );
		x::variable_symbol * add_variable( x::variable_decl_ast * val );
		x::template_symbol * add_template( x::template_decl_ast * val );
		x::namespace_symbol * add_namespace( x::namespace_decl_ast * val );
		x::foundation_symbol * add_foundation( std::string_view name, x::uint64 size );
		x::nativefunc_symbol * add_nativefunc( std::string_view name, void * callback );
		x::builtinfunc_symbol * add_builtinfunc( std::string_view name, x::builtin_ptr builtin );

	public:
		void push_scope( std::string_view name );
		void push_scope( x::scope_symbol * symbol );
		x::scope_symbol * cur_scope() const;
		void pop_scope();

	public:
		x::type_symbol * find_type_symbol( std::string_view name ) const;
		x::scope_symbol * find_scope_symbol( std::string_view name ) const;
		x::class_symbol * find_class_symbol( std::string_view name ) const;
		std::vector<x::template_symbol *> find_template_symbols( std::string_view name ) const;
		x::symbol * find_symbol( std::string_view name, x::scope_symbol * scope = nullptr ) const;
		x::symbol * up_find_symbol_from_type( x::symbol_t type ) const;

	public:
		x::namespace_symbol * global_namespace() const;
		std::string calc_fullname( std::string_view name ) const;
		x::symbol * find_symbol_from_ast( const x::ast_ptr & ast ) const;
		x::symbol * find_symbol_from_fullname( std::string_view fullname ) const;

	public:
		void add_reference( x::ast * ast, std::string_view name, x::symbol * val );
		x::symbol * find_reference( x::ast * ast, std::string_view name ) const;

	private:
		x::symbol * up_find_symbol( std::string_view name, x::scope_symbol * scope ) const;
		x::symbol * down_find_symbol( std::string_view name, x::scope_symbol * scope ) const;

	private:
		std::deque<x::scope_symbol *> _scope;
		std::map<x::ast_ptr, x::symbol *> _astmap;
		std::map<std::string, x::symbol *> _symbolmap;
		std::multimap<std::string, x::template_symbol *> _templatemap;
		std::map<x::ast *, std::map<std::string, x::symbol *>> _referencemap;
	};
}
