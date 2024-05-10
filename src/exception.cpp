#include "exception.h"

#include <format>

x::lexical_exception::lexical_exception( std::string_view message, const x::location & location )
{
	this->message = std::format( "[lexical error]{} {}:{}\t{}", location.file, location.line, location.column, message );
}

char const * x::lexical_exception::what() const
{
	return message.c_str();
}

x::grammatical_exception::grammatical_exception( std::string_view message, const x::location & location )
{
	this->message = std::format( "[grammatical error]{} {}:{}\t{}", location.file, location.line, location.column, message );
}

char const * x::grammatical_exception::what() const
{
	return message.c_str();
}

x::semantic_exception::semantic_exception( std::string_view message, const x::location & location )
{
	this->message = std::format( "[semantic error]{} {}:{}\t{}", location.file, location.line, location.column, message );
}

char const * x::semantic_exception::what() const
{
	return message.c_str();
}

x::bad_value_access::bad_value_access( std::string_view message )
	: message( message )
{
}

char const * x::bad_value_access::what() const
{
	return message.c_str();
}
