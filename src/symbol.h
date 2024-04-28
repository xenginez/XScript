#pragma once

#include <span>

#include "type.h"

namespace x
{
	class symbol : public std::enable_shared_from_this<symbol>
	{
	public:
		~symbol() = default;

	public:
		virtual bool is_scope() const
		{
			return false;
		}
		virtual symbol_ptr find_child_symbol( std::string_view name ) const
		{
			return nullptr;
		}

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
	class flag_symbol : public type_symbol
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
}
