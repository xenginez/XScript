#pragma once

#include <any>
#include <map>
#include <span>
#include <array>
#include <unordered_map>

#include "type.h"

namespace x
{
	class object
	{
	public:
		void * operator new( size_t size );
		void operator delete( void * ptr );

	public:
		virtual bool is_any() const;
		virtual bool is_array() const;
		virtual bool is_native() const;
		virtual bool is_script() const;
		virtual bool is_closure() const;
		virtual meta_class_ptr meta_class() const;

	public:
		virtual void * data() const;
		virtual uint64_t size() const;

	public:
		virtual void assign( const void * val );
		virtual int compare( const void * val );
		virtual std::string format( std::string_view fmt );
	};

	class any_object : public object
	{
	public:
		any_object() = default;

	public:
		any_object & assign( const std::any & val );

	private:
		std::any _data;
	};

	class array_object : public object
	{
	public:
		array_object() = default;

	public:
		template<typename ... T> array_object & assign( const std::vector<T...> & val );
		template<typename T, size_t N> array_object & assign( const std::span<T, N> & val );
		template<typename T, size_t N> array_object & assign( const std::array<T, N> & val );

	public:
		template<typename T> T to_span() const;
		template<typename T> T to_vector() const;

	private:
		std::vector<value> _data;
	};

	class string_object : public object
	{
	public:
		string_object() = default;

	public:
		string_object & assign( const char * str );
		string_object & assign( std::string_view str );
		string_object & assign( const std::string & str );

	public:
		const char * c_str() const;
		const std::string & string() const;
		std::string_view string_view() const;

	private:
		std::string _data;
	};

	class script_object : public object
	{
	public:
		script_object( uint64_t code, void * data = nullptr );

	public:

	private:
		void * _data;
		uint64_t _hashcode;
	};

	class closure_object : public object
	{
	public:
		closure_object() = default;

	public:

	private:
	};

	class coroutines_object : public object
	{
	public:
		coroutines_object() = default;

	public:

	private:
	};

	template<typename T> class native_object : public object
	{
	public:
		native_object() = default;

	public:

	private:
		T _data;
	};
}
