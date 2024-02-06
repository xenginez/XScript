#pragma once

#include <map>
#include <span>

#include "runtime.h"

namespace x
{
	class meta : public std::enable_shared_from_this<meta>
	{
		friend class context;

	public:
		virtual ~meta() = default;

	public:
		meta_t type() const;
		uint64_t hashcode() const;
		x::static_string_view name() const;
		x::static_string_view fullname() const;

	protected:
		x::meta_t _type;
		uint64_t _hashcode;
		x::static_string_view _name;
		x::static_string_view _fullname;
	};

	class meta_namespace : public meta
	{
		friend class context;

	public:
		meta_namespace();

	public:
		std::span<x::meta_ptr> members() const;

	protected:
		mutable std::vector<x::meta_ptr> _members;
	};

	class meta_function : public meta
	{
		friend class context;

	public:
		meta_function();

	public:
		x::access_t access() const;
		x::modify_t modify() const;
		x::type_desc result_type() const;
		std::span<x::type_desc> parameter_types() const;

	public:
		virtual void invoke() const = 0;

	protected:
		x::access_t _access;
		x::modify_t _modify;
		x::type_desc _result_type;
		mutable std::vector<x::type_desc> _parameter_types;
	};

	class meta_variable : public meta
	{
		friend class context;

	public:
		meta_variable();

	public:
		x::access_t access() const;
		x::modify_t modify() const;
		x::type_desc value_type() const;

	public:
		virtual void get() const = 0;
		virtual void set() const = 0;

	protected:
		x::access_t _access;
		x::modify_t _modify;
		x::type_desc _value_type;
	};

	class meta_class : public meta
	{
		friend class context;

	public:
		meta_class();

	public:
		uint64_t class_size() const;
		x::static_string_view class_base() const;
		std::span<x::meta_variable_ptr> variables() const;
		std::span<x::meta_function_ptr> functions() const;

	public:
		virtual void construct( void * ptr ) const = 0;

	protected:
		uint64_t _size;
		x::static_string_view _base;
		mutable std::vector<x::meta_variable_ptr> _variables;
		mutable std::vector<x::meta_function_ptr> _functions;
	};

	class meta_enum : public meta
	{
		friend class context;

	public:
		meta_enum();

	public:
		std::span<std::pair<x::static_string_view, x::value>> elements() const;

	protected:
		mutable std::vector<std::pair<x::static_string_view, x::value>> _elements;
	};


	template<typename C, typename R, typename ... As> class meta_native_const_function : public meta_function
	{
		friend class context;

	public:
		using class_type = C;
		using result_type = R;
		using parameter_type = std::tuple<std::remove_reference_t<As>...>;
		using function_type = result_type( class_type:: * )( As... ) const;

	public:
		void invoke() const override
		{
			class_type * obj = x::runtime::pop().get<class_type *>();

			parameter_type parameters = std::make_tuple( x::runtime::pop().get<As>()... );

			if constexpr ( std::is_void_v<result_type> )
			{
				std::apply( _func, std::tuple_cat( std::make_tuple( obj ), parameters ) );
			}
			else
			{
				x::runtime::push( std::apply( _func, std::tuple_cat( std::make_tuple( obj ), parameters ) ) );
			}
		}

	private:
		function_type _func;
	};

	template<typename C, typename R, typename ... As> class meta_native_function : public meta_function
	{
		friend class context;

	public:
		using class_type = C;
		using result_type = R;
		using parameter_type = std::tuple<std::remove_reference_t<As>...>;
		using function_type = result_type( class_type::* )( As... );

	public:
		void invoke() const override
		{
			class_type * obj = x::runtime::pop().get<class_type *>();

			parameter_type parameters = std::make_tuple( x::runtime::pop().get<As>()... );

			if constexpr ( std::is_void_v<result_type> )
			{
				std::apply( _func, std::tuple_cat( std::make_tuple( obj ), parameters ) );
			}
			else
			{
				x::runtime::push( std::apply( _func, std::tuple_cat( std::make_tuple( obj ), parameters ) ) );
			}
		}

	private:
		function_type _func;
	};

	template<typename R, typename ... As> class meta_native_static_function : public meta_function
	{
		friend class context;

	public:
		using result_type = R;
		using parameter_type = std::tuple<std::remove_reference_t<As>...>;
		using function_type = result_type( * )( As... );

	public:
		void invoke() const override
		{
			parameter_type parameters = std::make_tuple( x::runtime::pop().get<As>()... );

			if constexpr ( std::is_void_v<result_type> )
			{
				std::apply( _func, parameters );
			}
			else
			{
				x::runtime::push( std::apply( _func, parameters ) );
			}
		}

	private:
		function_type _func;
	};

	template<typename C, typename T> class meta_native_variable : public meta_variable
	{
		friend class context;

	public:
		using class_type = C;
		using value_type = T;
		using variable_type = value_type class_type:: *;

	public:
		void get() const override
		{
			class_type * obj = x::runtime::pop().get<class_type *>();

			x::runtime::push( obj->*_var );
		}

		void set() const override
		{
			class_type * obj = x::runtime::pop().get<class_type *>();

			if constexpr ( std::is_pointer_v<value_type> )
				obj->*_var = x::runtime::pop().get<value_type>();
			else
				obj->*_var = *x::runtime::pop().get<std::decay_t<value_type> *>();
		}

	private:
		variable_type _var;
	};

	template<typename T> class meta_native_static_variable : public meta_variable
	{
		friend class context;

	public:
		using value_type = T;
		using variable_type = value_type *;

	public:
		void get() const override
		{
			x::runtime::push( *_var );
		}

		void set() const override
		{
			*_var = x::runtime::pop().get<const value_type &>();
		}

	private:
		variable_type _var;
	};

	template<typename T> class meta_native_class : public meta_class
	{
		friend class context;

	public:
		using class_type = T;
		using this_type = meta_native_class<T>;

	public:
		void construct( void * ptr ) const override
		{
			new ( ptr ) class_type();
		}
	};

	template<typename T> class meta_native_enum : public meta_enum
	{
		friend class context;

	public:
		using enum_type = T;
		using this_type = meta_native_enum<T>;
	};


	class meta_extern_function : public meta_function
	{
		friend class context;

	public:
		void invoke() const override;

	private:
		x::static_string_view _lib;
		x::static_string_view _proc;
	};


	class meta_script_function : public meta_function
	{
		friend class context;

	public:
		void invoke() const override;

	private:
		code _code;
	};

	class meta_script_variable : public meta_variable
	{
		friend class context;

	public:
		void get() const override;

		void set() const override;

	private:
		uint64_t _idx;
	};

	class meta_script_class : public meta_class
	{
		friend class context;

	public:
		void construct( void * ptr ) const override;

	private:
		code _code;
	};
}
