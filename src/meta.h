#pragma once

#include <map>
#include <span>

#include "type.h"
#include "static_string_view.hpp"

namespace x
{
	class meta : public std::enable_shared_from_this<meta>
	{
		friend class context;

	public:
		virtual ~meta() = default;

	public:
		x::meta_t type() const;
		std::uint64_t hashcode() const;
		x::static_string_view name() const;
		x::static_string_view fullname() const;

	protected:
		x::meta_t _type;
		std::uint64_t _hashcode;
		x::static_string_view _name;
		x::static_string_view _fullname;
	};

	class meta_enum : public meta
	{
		friend class context;

	public:
		meta_enum();

	public:
		std::span<const std::pair<x::static_string_view, std::int64_t>> elements() const;

	protected:
		std::vector<std::pair<x::static_string_view, std::int64_t>> _elements;
	};

	class meta_class : public meta
	{
		friend class context;

	public:
		meta_class();

	public:
		std::uint64_t class_size() const;
		x::static_string_view class_base() const;
		std::span<const x::meta_variable_ptr> variables() const;
		std::span<const x::meta_function_ptr> functions() const;

	public:
		virtual void construct( void * ptr ) const = 0;

	protected:
		std::uint64_t _size = 0;
		x::static_string_view _base;
		std::vector<x::meta_variable_ptr> _variables;
		std::vector<x::meta_function_ptr> _functions;
	};

	class meta_function : public meta
	{
		friend class context;

	public:
		meta_function();

	public:
		x::access_t access() const;
		x::modify_flag modify() const;
		x::type_desc result_type() const;
		std::span<const x::type_desc> parameter_types() const;

	public:
		virtual void invoke() const = 0;

	protected:
		x::access_t _access = x::access_t::PRIVATE;
		x::modify_flag _modify = x::modify_flag::NONE;
		x::type_desc _result_type;
		std::vector<x::type_desc> _parameter_types;
	};

	class meta_variable : public meta
	{
		friend class context;

	public:
		meta_variable();

	public:
		x::access_t access() const;
		x::modify_flag modify() const;
		x::type_desc value_type() const;

	public:
		virtual void get() const = 0;
		virtual void set() const = 0;

	protected:
		x::access_t _access = x::access_t::PRIVATE;
		x::modify_flag _modify = x::modify_flag::NONE;
		x::type_desc _value_type;
	};

	class meta_namespace : public meta
	{
		friend class context;

	public:
		meta_namespace();

	public:
		std::span<const x::meta_ptr> members() const;

	protected:
		std::vector<x::meta_ptr> _members;
	};

	class meta_script_enum : public meta_enum
	{
		friend class context;

	public:
		meta_script_enum();
	};

	class meta_script_class : public meta_class
	{
		friend class context;

	public:
		meta_script_class();

	public:
		void construct( void * ptr ) const override;

	private:
		code _code = {};
	};

	class meta_script_function : public meta_function
	{
		friend class context;

	public:
		meta_script_function();

	public:
		void invoke() const override;

	private:
		code _code = {};
	};

	class meta_script_variable : public meta_variable
	{
		friend class context;

	public:
		meta_script_variable();

	public:
		void get() const override;

		void set() const override;

	private:
		std::uint64_t _idx = 0;
	};

	class meta_extern_function : public meta_function
	{
		friend class context;

	public:
		meta_extern_function();

	public:
		void invoke() const override;

	private:
		x::static_string_view _lib;
		x::static_string_view _proc;
	};
}
