#pragma once

#include <map>
#include <span>
#include <deque>

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
		bool is_type() const;
		bool is_scope() const;
		bool is_variable() const;
		bool is_function() const;

	public:
		x::type_symbol * cast_type();
		x::scope_symbol * cast_scope();

	public:
		x::symbol_t type = x::symbol_t::UNIT;
		x::ast_ptr ast;
		std::string name;
		std::string fullname;
		x::symbol * parent = nullptr;
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

	class unit_symbol : public symbol, public scope_symbol
	{
	public:
		unit_symbol();
		~unit_symbol() override;

	public:
		x::unit_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::symbol *> children;
	};
	class enum_symbol : public symbol, public type_symbol, public scope_symbol
	{
	public:
		enum_symbol();
		~enum_symbol() override;

	public:
		x::enum_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::element_symbol *> elements;
	};
	class alias_symbol : public symbol, public type_symbol
	{
	public:
		alias_symbol();
		~alias_symbol() override;

	public:
		x::using_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;

	public:
		x::type_symbol * retype = nullptr;
	};
	class class_symbol : public symbol, public type_symbol, public scope_symbol
	{
	public:
		class_symbol();
		~class_symbol() override;

	public:
		x::class_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		std::vector<x::alias_symbol *> aliases;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class block_symbol : public symbol, public scope_symbol
	{
	public:
		block_symbol();
		~block_symbol() override;

	public:
		x::compound_stat_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::local_symbol *> locals;
		std::vector<x::block_symbol *> blocks;
	};
	class cycle_symbol : public block_symbol
	{
	public:
		cycle_symbol();
		~cycle_symbol() override;

	public:
		x::cycle_stat_ast_ptr cast_ast() const;
	};
	class local_symbol : public symbol
	{
	public:
		local_symbol();
		~local_symbol() override;

	public:
		x::local_stat_ast_ptr cast_ast() const;

	public:
		x::type_symbol * valuetype = nullptr;
	};
	class param_symbol : public symbol
	{
	public:
		param_symbol();
		~param_symbol() override;

	public:
		x::parameter_decl_ast_ptr cast_ast() const;

	public:
		x::type_symbol * valuetype = nullptr;
	};
	class element_symbol : public symbol
	{
	public:
		element_symbol();
		~element_symbol() override;

	public:
		x::element_decl_ast_ptr cast_ast() const;
	};
	class function_symbol : public symbol, public scope_symbol
	{
	public:
		function_symbol();
		~function_symbol() override;

	public:
		x::function_decl_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::type_symbol * result = nullptr;
		x::block_symbol * block = nullptr;
		std::vector<x::param_symbol *> parameters;
	};
	class variable_symbol : public symbol
	{
	public:
		variable_symbol();
		~variable_symbol() override;

	public:
		x::variable_decl_ast_ptr cast_ast() const;

	public:
		x::type_symbol * value = nullptr;
	};
	class template_symbol : public symbol, public type_symbol, public scope_symbol
	{
	public:
		template_symbol();
		~template_symbol() override;

	public:
		x::template_decl_ast_ptr cast_ast() const;

	public:
		x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		std::vector<x::alias_symbol *> aliases;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class namespace_symbol : public symbol, public scope_symbol
	{
	public:
		namespace_symbol();
		~namespace_symbol() override;

	public:
		x::namespace_decl_ast_ptr cast_ast() const;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::type_symbol *> children;
	};

	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void push_scope( std::string_view name );

		x::unit_symbol * add_unit( x::unit_ast * ast );
		x::enum_symbol * add_enum( x::enum_decl_ast * ast );
		x::alias_symbol * add_alias( x::using_decl_ast * ast );
		x::class_symbol * add_class( x::class_decl_ast * ast );
		x::block_symbol * add_block( x::compound_stat_ast * ast );
		x::cycle_symbol * add_cycle( x::cycle_stat_ast * ast );
		x::local_symbol * add_local( x::local_stat_ast * ast );
		x::param_symbol * add_param( x::parameter_decl_ast * ast );
		x::element_symbol * add_element( x::element_decl_ast * ast );
		x::function_symbol * add_function( x::function_decl_ast * ast );
		x::variable_symbol * add_variable( x::variable_decl_ast * ast );
		x::template_symbol * add_template( x::template_decl_ast * ast );
		x::namespace_symbol * add_namespace( x::namespace_decl_ast * ast );

		x::type_symbol * find_type_symbol( std::string_view name ) const;
		x::scope_symbol * find_scope_symbol( std::string_view name ) const;
		x::class_symbol * find_class_symbol( std::string_view name ) const;
		x::symbol * find_symbol( std::string_view name, x::scope_symbol * scope = nullptr ) const;

		x::scope_symbol * current_scope() const;

		void pop_scope();

	public:
		x::namespace_symbol * global_namespace() const;
		std::string calc_fullname( std::string_view name ) const;
		x::symbol * find_symbol_from_fullname( std::string_view fullname ) const;

	public:
		void add_reference( const x::location & location, x::symbol * val );
		x::symbol * find_reference( const x::location & location ) const;

	private:
		void add_symbol( x::symbol * val );
		x::symbol * up_find_symbol( std::string_view name, x::scope_symbol * scope ) const;
		x::symbol * down_find_symbol( std::string_view name, x::scope_symbol * scope ) const;

	private:
		std::deque<x::scope_symbol *> _scope;
		std::map<std::string, x::symbol *> _symbolmap;
		std::map<std::string, x::symbol *> _referencemap;
	};
}
