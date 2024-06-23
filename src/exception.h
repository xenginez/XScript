#pragma once

#include <exception>

#include "type.h"

namespace x
{
	class lexical_exception : public std::exception
	{
	public:
		lexical_exception( std::string_view message, const x::location & location );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class syntax_exception : public std::exception
	{
	public:
		syntax_exception( std::string_view message, const x::location & location );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class semantic_exception : public std::exception
	{
	public:
		semantic_exception( std::string_view message );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class compile_exception : public std::exception
	{
	public:
		compile_exception( std::string_view message );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class bad_value_access : public std::exception
	{
	public:
		bad_value_access( std::string_view message );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class runtime_exception : public std::exception
	{
	public:
		runtime_exception( std::string_view message );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
}