#pragma once

#include <exception>

#include "type.h"

namespace x
{
	class lexical_exception : public std::exception
	{
	public:
		lexical_exception( std::string_view message, const x::source_location & location );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class grammatical_exception : public std::exception
	{
	public:
		grammatical_exception( std::string_view message, const x::source_location & location );

	public:
		char const * what() const override;

	private:
		std::string message;
	};
	class semantic_exception : public std::exception
	{
	public:
		semantic_exception( std::string_view message, const x::source_location & location );

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
}