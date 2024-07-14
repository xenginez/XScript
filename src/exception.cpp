#include "exception.h"

#include <format>

x::lexical_exception::lexical_exception( std::string_view message, const x::location & location )
{
	this->message = std::format( "[lexical error]{} {}:{}\t{}", location.file, location.line, location.col, message );
}

char const * x::lexical_exception::what() const
{
	return message.c_str();
}

x::syntax_exception::syntax_exception( std::string_view message, const x::location & location )
{
	this->message = std::format( "[grammatical error]{} {}:{}\t{}", location.file, location.line, location.col, message );
}

char const * x::syntax_exception::what() const
{
	return message.c_str();
}

x::semantic_exception::semantic_exception( std::string_view message )
{
	this->message = std::format( "[semantic error]{}", message );
}

char const * x::semantic_exception::what() const
{
	return message.c_str();
}

x::compile_exception::compile_exception( std::string_view message )
{
	this->message = std::format( "[compile error]{}", message );
}

char const * x::compile_exception::what() const
{
	return message.c_str();
}

x::bad_value_access::bad_value_access( std::string_view message )
{
	this->message = std::format( "[bad value error]{}", message );
}

char const * x::bad_value_access::what() const
{
	return message.c_str();
}

x::runtime_exception::runtime_exception( std::string_view message )
{
	this->message = std::format( "[runtime error]{}", message );
}

char const * x::runtime_exception::what() const
{
	return message.c_str();
}
