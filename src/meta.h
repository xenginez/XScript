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
		virtual x::meta_t type() const = 0;
		virtual std::size_t hashcode() const = 0;
		virtual x::static_string_view name() const = 0;
		virtual x::static_string_view fullname() const = 0;
	};

	class meta_type : public meta
	{
		friend class context;

	public:
		virtual std::uint64_t size() const = 0;
	};

	class meta_enum : public meta_type
	{
		friend class context;

	public:
		meta_enum();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		std::uint64_t size() const override;

	public:
		std::span<const std::pair<x::static_string_view, x::int64>> elements() const;

	private:
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<std::pair<x::static_string_view, x::int64>> _elements;
	};

	class meta_flag : public meta_type
	{
		friend class context;

	public:
		meta_flag();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		std::uint64_t size() const override;

	public:
		std::span<const std::pair<x::static_string_view, x::uint64>> elements() const;

	private:
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<std::pair<x::static_string_view, x::uint64>> _elements;
	};

	class meta_class : public meta_type
	{
		friend class context;

	public:
		meta_class();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		std::uint64_t size() const override;

	public:
		x::static_string_view base() const;
		std::span<const x::meta_variable_ptr> variables() const;
		std::span<const x::meta_function_ptr> functions() const;

	public:
		void construct( void * ptr ) const;

	private:
		std::uint64_t _size = 0;
		x::static_string_view _base;
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::uint32_t _construct_code_idx = 0;
		std::uint32_t _construct_code_size = 0;
		std::vector<x::meta_variable_ptr> _variables;
		std::vector<x::meta_function_ptr> _functions;
	};

	class meta_function : public meta
	{
		friend class context;

	public:
		meta_function();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		x::access_t access() const;
		x::modify_flag modify() const;
		x::type_desc result_type() const;
		std::span<const x::type_desc> parameter_types() const;

	public:
		void invoke() const;

	private:
		x::access_t _access = x::access_t::PRIVATE;
		x::modify_flag _modify = x::modify_flag::NONE;
		std::uint32_t _code_idx = 0;
		std::uint32_t _code_size = 0;
		x::type_desc _result_type;
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::type_desc> _parameter_types;
	};

	class meta_variable : public meta
	{
		friend class context;

	public:
		meta_variable();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		x::access_t access() const;
		x::modify_flag modify() const;
		x::type_desc value_type() const;

	public:
		void get( const x::value & obj ) const;
		void set( const x::value & obj ) const;

	private:
		std::uint64_t _idx = 0;
		x::access_t _access = x::access_t::PRIVATE;
		x::modify_flag _modify = x::modify_flag::NONE;
		x::type_desc _value_type;
		x::static_string_view _name;
		x::static_string_view _fullname;
	};

	class meta_namespace : public meta
	{
		friend class context;

	public:
		meta_namespace();

	public:
		x::meta_t type() const override;
		std::size_t hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		std::span<const x::meta_type_ptr> members() const;

	private:
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::meta_type_ptr> _members;
	};
}
