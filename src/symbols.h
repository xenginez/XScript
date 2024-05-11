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

	public:
		x::type_symbol * cast_type();
		x::scope_symbol * cast_scope();

	public:
		x::symbol_t type = x::symbol_t::UNIT;
		std::string name;
		std::string fullname;
		x::location location;
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
		virtual x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<enum_element_symbol *> elements;
	};
	class flag_symbol :public symbol, public type_symbol, public scope_symbol
	{
	public:
		flag_symbol();
		~flag_symbol() override;

	public:
		virtual x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<flag_element_symbol *> elements;
	};
	class alias_symbol : public symbol, public type_symbol
	{
	public:
		alias_symbol();
		~alias_symbol() override;

	public:
		virtual x::uint64 size() const override;

	public:
		x::typedesc desc;
	};
	class class_symbol : public symbol, public type_symbol, public scope_symbol
	{
	public:
		class_symbol();
		~class_symbol() override;

	public:
		virtual x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
	};
	class block_symbol : public symbol, public scope_symbol
	{
	public:
		block_symbol();
		~block_symbol() override;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::symbol *> children;
	};
	class cycle_symbol : public block_symbol
	{
	public:
		cycle_symbol();
		~cycle_symbol() override;
	};
	class local_symbol : public symbol
	{
	public:
		local_symbol();
		~local_symbol() override;

	public:
		x::type_symbol * value = nullptr;
	};
	class param_symbol : public symbol
	{
	public:
		param_symbol();
		~param_symbol() override;

	public:
		x::type_symbol * value = nullptr;
	};
	class function_symbol : public symbol, public scope_symbol
	{
	public:
		function_symbol();
		~function_symbol() override;

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
		x::uint64 idx = 0;
		x::type_symbol * value = nullptr;
	};
	class template_symbol : public symbol, public type_symbol, public scope_symbol
	{
	public:
		template_symbol();
		~template_symbol() override;

	public:
		virtual x::uint64 size() const override;
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		x::class_symbol * base = nullptr;
		std::vector<x::function_symbol *> functions;
		std::vector<x::variable_symbol *> variables;
		std::vector<x::temp_element_symbol *> elements;
	};
	class namespace_symbol : public symbol, public scope_symbol
	{
	public:
		namespace_symbol();
		~namespace_symbol() override;

	public:
		void add_child( x::symbol * val ) override;
		x::symbol * find_child( std::string_view name ) const override;

	public:
		std::vector<x::type_symbol *> children;
	};
	class enum_element_symbol : public symbol
	{
	public:
		enum_element_symbol();
		~enum_element_symbol() override;

	public:
		x::int64 value = 0;
	};
	class flag_element_symbol : public symbol
	{
	public:
		flag_element_symbol();
		~flag_element_symbol() override;

	public:
		x::uint64 value = 0;
	};
	class temp_element_symbol : public symbol
	{
	public:
		temp_element_symbol();
		~temp_element_symbol() override;

	public:
		x::typedesc desc;
	};

	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void push_scope( std::string_view name );
		x::unit_symbol * add_unit( const x::location & location );
		x::enum_symbol * add_enum( std::string_view name, const x::location & location );
		x::flag_symbol * add_flag( std::string_view name, const x::location & location );
		x::alias_symbol * add_alias( std::string_view name, const x::typedesc & desc, const x::location & location );
		x::class_symbol * add_class( std::string_view name, std::string_view base, const x::location & location );
		x::block_symbol * add_block( const x::location & location );
		x::cycle_symbol * add_cycle( const x::location & location );
		x::local_symbol * add_local( std::string_view name, std::string_view type, const x::location & location );
		x::param_symbol * add_param( std::string_view name, std::string_view type, const x::location & location );
		x::function_symbol * add_function( std::string_view name, std::string_view type, const x::location & location );
		x::variable_symbol * add_variable( std::string_view name, std::string_view type, const x::location & location );
		x::template_symbol * add_template( std::string_view name, std::string_view base, const x::location & location );
		x::namespace_symbol * add_namespace( std::string_view name, const x::location & location );
		x::enum_element_symbol * add_enum_element( std::string_view name, x::int64 value, const x::location & location );
		x::flag_element_symbol * add_flag_element( std::string_view name, x::uint64 value, const x::location & location );
		x::temp_element_symbol * add_temp_element( std::string_view name, x::typedesc desc, const x::location & location );
		bool has_symbol( std::string_view name ) const;
		x::type_symbol * find_type_symbol( std::string_view name ) const;
		x::scope_symbol * find_scope_symbol( std::string_view name ) const;
		x::class_symbol * find_class_symbol( std::string_view name ) const;
		x::symbol * find_symbol( std::string_view name, x::scope_symbol * scope = nullptr ) const;
		x::symbol * up_find_symbol( std::string_view name, x::scope_symbol * scope = nullptr ) const;
		x::symbol * down_find_symbol( std::string_view name, x::scope_symbol * scope = nullptr ) const;
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
		std::deque<x::scope_symbol *> _scope;
		std::map<std::string, x::symbol *> _symbolmap;
		std::map<std::string, x::symbol *> _referencemap;
	};
}