#pragma once

#include <map>
#include <span>
#include <deque>

#include "type.h"

namespace x
{
	class symbol : public std::enable_shared_from_this<symbol>
	{
	public:
		~symbol() = default;

	public:
		virtual bool is_scope() const;
		virtual symbol_ptr find_child_symbol( std::string_view name ) const;

	public:
		x::symbol_t type;
		std::string name;
		x::location location;
		std::weak_ptr<symbol> parent;
	};
	class type_symbol : public symbol
	{
	public:
		virtual x::uint64 size() const;

	public:
		int array = 0;
		bool is_ref = false;
		bool is_const = false;
	};
	class enum_symbol : public type_symbol
	{
	public:
		x::uint64 size() const override;
		std::span<std::string_view> elements() const;
	};
	class flag_symbol : public type_symbol
	{
	public:
		x::uint64 size() const override;
		std::span<std::string_view> elements() const;
	};
	class alias_symbol : public type_symbol
	{
	public:
		x::uint64 size() const override;

	public:
		x::typedesc desc;
	};
	class class_symbol : public type_symbol
	{
	public:
		x::uint64 size() const override;
		class_symbol_ptr base() const;
		std::span<function_symbol_ptr> functions() const;
		std::span<variable_symbol_ptr> variables() const;
	};
	class block_symbol : public symbol
	{
	public:
		std::span<symbol_ptr> symbols() const;
	};
	class function_symbol : public symbol
	{
	public:
		block_symbol_ptr block() const;
		type_symbol_ptr result() const;
		std::span<variable_symbol_ptr> parameters() const;
	};
	class variable_symbol : public symbol
	{
	public:
		type_symbol_ptr valuetype() const;
	};
	class template_symbol : public symbol
	{
	public:

	};
	class namespace_symbol : public symbol
	{
	public:
		std::span<type_symbol_ptr> types() const;
	};
	class enum_element_symbol : public symbol
	{
	public:
		x::int64 value;
	};
	class flag_element_symbol : public symbol
	{
	public:
		x::uint64 value;
	};

	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		symbols();
		~symbols();

	public:
		void push_unit( const x::location & location );
		void pop_unit();

		void push_scope( const x::location & location );
		void add_symbol( const x::symbol_ptr & val );
		bool has_symbol( std::string_view symbol_name ) const;
		x::symbol_ptr current_scope() const;
		void pop_scope();

	public:
		x::namespace_symbol_ptr global_namespace() const;
		x::symbol_ptr find_symbol_from_name( std::string_view symbol_name ) const;
		x::symbol_ptr find_symbol_from_fullname( std::string_view symbol_fullname ) const;

	public:
		void add_reference( const x::location & location, const x::symbol_ptr & val );
		x::symbol_ptr find_reference( const x::location & location ) const;

	private:
		x::namespace_symbol_ptr _global;
		std::map<std::string, symbols_ptr> _symbolmap;
		std::map<std::string, symbols_ptr> _referencemap;
	};
}