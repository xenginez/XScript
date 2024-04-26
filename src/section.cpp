#include "section.h"

x::section_t x::type_section::type() const
{
	return x::section_t::TYPE;
}

void x::type_section::load( std::istream & in )
{
}

void x::type_section::save( std::ostream & out ) const
{
}

x::section_t x::depend_section::type() const
{
	return x::section_t::DEPEND;
}

void x::depend_section::load( std::istream & in )
{
}

void x::depend_section::save( std::ostream & out ) const
{
}

x::section_t x::global_section::type() const
{
	return x::section_t::GLOBAL;
}

void x::global_section::load( std::istream & in )
{
}

void x::global_section::save( std::ostream & out ) const
{
}

x::section_t x::function_section::type() const
{
	return x::section_t::FUNCTION;
}

void x::function_section::load( std::istream & in )
{
}

void x::function_section::save( std::ostream & out ) const
{
}

x::section_t x::variable_section::type() const
{
	return x::section_t::VARIABLE;
}

void x::variable_section::load( std::istream & in )
{
}

void x::variable_section::save( std::ostream & out ) const
{
}

x::section_t x::codedata_section::type() const
{
	return x::section_t::CODEDATA;
}

void x::codedata_section::load( std::istream & in )
{
}

void x::codedata_section::save( std::ostream & out ) const
{
}

x::section_t x::stringdata_section::type() const
{
	return x::section_t::STRINGDATA;
}

void x::stringdata_section::load( std::istream & in )
{
}

void x::stringdata_section::save( std::ostream & out ) const
{
}

x::section_t x::customdata_section::type() const
{
	return x::section_t::CUSTOMDATA;
}

void x::customdata_section::load( std::istream & in )
{
}

void x::customdata_section::save( std::ostream & out ) const
{
}
