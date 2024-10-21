#pragma once

#include "type.h"

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
		virtual std::string_view name() const = 0;
		virtual std::string_view fullname() const = 0;

	public:
		std::string_view attribute( std::string_view key ) const;

	private:
		x::meta_attribute * _attribute;
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
		struct element
		{
			x::int64 value;
			std::string_view name;
		};

	public:
		meta_enum();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		std::span<const element> elements() const;

	private:
		std::string_view _name;
		std::string_view _fullname;
		std::vector<element> _elements;
	};

	class meta_class : public meta_type
	{
		friend class context;

	public:
		meta_class();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		const x::meta_class * base() const;
		std::span<const x::meta_variable * const> variables() const;
		std::span<const x::meta_function * const> functions() const;
		std::span<const x::meta_interface * const> interfaces() const;

	private:
		x::uint64 _size = 0;
		x::uint64 _construct = 0;
		std::string_view _name;
		std::string_view _fullname;
		const x::meta_class * _base = nullptr;
		std::vector<const x::meta_variable *> _variables;
		std::vector<const x::meta_function *> _functions;
		std::vector<const x::meta_interface *> _interfaces;
	};

	class meta_template : public meta_type
	{
		friend class context;

	public:
		meta_template();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		const x::meta_class * base() const;
		std::span<const x::meta_variable * const> variables() const;
		std::span<const x::meta_function * const> functions() const;
		std::span<const x::meta_interface * const> interfaces() const;

	private:
		x::uint64 _size = 0;
		x::uint64 _construct = 0;
		std::string_view _name;
		std::string_view _fullname;
		const x::meta_class * _base;
		std::vector<const x::meta_variable *> _variables;
		std::vector<const x::meta_function *> _functions;
		std::vector<const x::meta_interface *> _interfaces;
	};

	class meta_variable : public meta
	{
		friend class context;

	public:
		meta_variable();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		bool is_static() const;
		bool is_thread() const;
		x::access_t access() const;
		const x::meta_type * value_type() const;

	public:
		bool get( const x::value & obj, x::value & val ) const;
		bool set( const x::value & obj, const x::value & val ) const;

	private:
		bool _is_static = false;
		bool _is_thread = false;
		x::uint64 _idx = 0;
		x::access_t _access = x::access_t::PRIVATE;
		std::string_view _name;
		std::string_view _fullname;
		const x::meta_type * _valuetype = nullptr;
	};

	class meta_function : public meta
	{
		friend class context;

	public:
		struct parameter
		{
			std::string_view _name;
			const x::meta_type * _valuetype = nullptr;
		};

	public:
		meta_function();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		bool is_const() const;
		bool is_async() const;
		bool is_static() const;
		x::access_t access() const;
		std::span<const parameter> parameters() const;
		std::span<const x::meta_type * const> results() const;

	public:
		void invoke() const;

	private:
		bool _is_const = false;
		bool _is_async = false;
		bool _is_static = false;
		x::access_t _access = x::access_t::PRIVATE;
		x::uint64 _code = 0;
		std::string_view _name;
		std::string_view _fullname;
		std::vector<parameter> _parameters;
		std::vector<const x::meta_type *> _results;
	};

	class meta_interface : public meta_type
	{
		friend class context;

	public:
		meta_interface();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		x::uint64 size() const override;
		void construct( void * ptr ) const override;

	public:
		std::span<const x::meta_function * const> functions() const;

	private:
		std::string_view _name;
		std::string_view _fullname;
		std::vector<const x::meta_function *> _functions;
	};

	class meta_namespace : public meta
	{
		friend class context;

	public:
		meta_namespace();

	public:
		x::meta_t type() const override;
		x::uint64 hashcode() const override;
		std::string_view name() const override;
		std::string_view fullname() const override;

	public:
		std::span<const x::meta_type * const> members() const;

	private:
		std::string_view _name;
		std::string_view _fullname;
		std::vector<const x::meta_type *> _members;
	};

	class meta_attribute : public std::enable_shared_from_this<meta_attribute>
	{
	public:
		std::string_view find( std::string_view key ) const;

	private:
		std::map<std::string_view, std::string_view> _map;
	};
}
