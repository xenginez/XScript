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
		virtual x::uint64 hashcode() const = 0;
		virtual x::static_string_view name() const = 0;
		virtual x::static_string_view fullname() const = 0;

	public:
		x::static_string_view attribute( std::string_view key ) const;

	private:
		x::meta_attribute_ptr _attribute;
	};

	class meta_type : public meta
	{
		friend class context;

	public:
		virtual x::uint64 size() const = 0;
		virtual void construct( void * ptr ) const = 0;
	};

	class meta_enum : public meta_type
	{
		friend class context;

	public:
		meta_enum();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		std::span<const x::meta_element_ptr> elements() const;

	private:
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::meta_element_ptr> _elements;
	};

	class meta_class : public meta_type
	{
		friend class context;

	public:
		meta_class();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		x::static_string_view base() const;
		std::span<const x::meta_variable_ptr> variables() const;
		std::span<const x::meta_function_ptr> functions() const;

	private:
		x::uint64 _size = 0;
		x::uint64 _construct = 0;
		x::static_string_view _base;
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::meta_variable_ptr> _variables;
		std::vector<x::meta_function_ptr> _functions;
	};

	class meta_element : public meta
	{
		friend class context;

	public:
		meta_element();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		x::int64 value() const;

	private:
		x::int64 _value = 0;
		x::static_string_view _name;
		x::static_string_view _fullname;
	};

	class meta_variable : public meta
	{
		friend class context;

	public:
		meta_variable();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		bool is_static() const;
		bool is_thread() const;
		x::access_t access() const;
		x::typedesc value() const;

	public:
		void get( const x::value & obj ) const;
		void set( const x::value & obj ) const;

	private:
		bool _is_static = false;
		bool _is_thread = false;
		x::uint64 _idx = 0;
		x::access_t _access = x::access_t::PRIVATE;
		x::typedesc _value;
		x::static_string_view _name;
		x::static_string_view _fullname;
	};

	class meta_function : public meta
	{
		friend class context;

	public:
		meta_function();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		bool is_const() const;
		bool is_async() const;
		bool is_static() const;
		x::access_t access() const;
		x::typedesc result() const;
		std::span<const x::meta_parameter_ptr> parameters() const;

	public:
		void invoke() const;

	private:
		bool _is_const = false;
		bool _is_async = false;
		bool _is_static = false;
		x::access_t _access = x::access_t::PRIVATE;
		x::uint64 _code = 0;
		x::typedesc _result;
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::meta_parameter_ptr> _parameter_types;
	};

	class meta_parameter : public meta
	{
		friend class context;

	public:
		meta_parameter();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		const x::typedesc & desc() const;

	private:
		x::typedesc _type;
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
		x::uint64 hashcode() const override;
		x::static_string_view name() const override;
		x::static_string_view fullname() const override;

	public:
		std::span<const x::meta_type_ptr> members() const;

	private:
		x::static_string_view _name;
		x::static_string_view _fullname;
		std::vector<x::meta_type_ptr> _members;
	};

	class meta_attribute : public std::enable_shared_from_this<meta_attribute>
	{
	public:
		x::static_string_view find( std::string_view key ) const;

	private:
		std::map<std::string_view, x::static_string_view> _map;
	};
}
