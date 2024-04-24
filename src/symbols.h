#pragma once

#include <map>
#include <span>
#include <deque>

#include "type.h"

namespace x
{
	class symbols : public std::enable_shared_from_this<symbols>
	{
	public:
		class symbol; using symbol_ptr = std::shared_ptr<symbol>;
		class type_symbol; using type_symbol_ptr = std::shared_ptr<type_symbol>;
		class enum_symbol; using enum_symbol_ptr = std::shared_ptr<enum_symbol>;
		class block_symbol; using block_symbol_ptr = std::shared_ptr<block_symbol>;
		class alias_symbol; using alias_symbol_ptr = std::shared_ptr<alias_symbol>;
		class class_symbol; using class_symbol_ptr = std::shared_ptr<class_symbol>;
		class function_symbol; using function_symbol_ptr = std::shared_ptr<function_symbol>;
		class variable_symbol; using variable_symbol_ptr = std::shared_ptr<variable_symbol>;
		class template_symbol; using template_symbol_ptr = std::shared_ptr<template_symbol>;
		class namespace_symbol; using namespace_symbol_ptr = std::shared_ptr<namespace_symbol>;

	public:
		class symbol : public std::enable_shared_from_this<symbol>
		{
		public:
			~symbol() = default;

		public:
			virtual bool is_scope() const { return false; }
			virtual symbol_ptr find_child_symbol( std::string_view name ) const { return nullptr; }

		public:
			x::symbol_t type;
			std::string name;
			x::source_location location;
			std::weak_ptr<symbol> parent;
		};
		class type_symbol : public symbol
		{
		public:
			x::modify_flag modify = x::modify_flag::NONE;
		};
		class enum_symbol : public type_symbol
		{
		public:
			std::span<std::string_view> elements() const;
		};
		class block_symbol : public symbol
		{
		public:
			std::span<symbol_ptr> symbols() const;
		};
		class alias_symbol : public type_symbol
		{
		public:
			x::type_desc desc;
		};
		class class_symbol : public type_symbol
		{
		public:
			class_symbol_ptr base() const;
			std::span<function_symbol_ptr> functions() const;
			std::span<variable_symbol_ptr> variables() const;
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

	public:
		symbols();
		~symbols();

	public:
		void push_unit( const x::source_location & location );
		void pop_unit();

		void push_scope( const x::source_location & location );
		void add_symbol( const symbol_ptr & val );
		bool has_symbol( std::string_view symbol_name ) const;
		void pop_scope();

	public:
		namespace_symbol_ptr global_namespace() const;
		symbol_ptr find_symbol_from_name( std::string_view symbol_name ) const;
		symbol_ptr find_symbol_from_fullname( std::string_view symbol_fullname ) const;

	public:
		void add_reference( const x::source_location & location, const symbol_ptr & val );
		symbol_ptr find_reference( const x::source_location & location ) const;

	private:
		namespace_symbol_ptr _global;
		std::map<std::string, symbols_ptr> _symbolmap;
		std::map<std::string, symbols_ptr> _referencemap;
	};
}