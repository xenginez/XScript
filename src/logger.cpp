#include "logger.h"

#include <iostream>

void x::logger::info( std::string_view msg, std::string_view file, int line, int col )
{
	if ( file.empty() )
		std::cout << std::format( "[INFO] {}", msg ) << std::endl;
	else
		std::cout << std::format( "[INFO] {} {}:{}\n\t{}", file, line, col, msg ) << std::endl;
}

void x::logger::error( std::string_view msg, std::string_view file, int line, int col )
{
	if ( file.empty() )
		std::cout << std::format( "[ERROR] {}", msg ) << std::endl;
	else
		std::cout << std::format( "[ERROR] {} {}:{}\n\t{}", file, line, col, msg ) << std::endl;
}

void x::logger::warning( std::string_view msg, std::string_view file, int line, int col )
{
	if ( file.empty() )
		std::cout << std::format( "[WARNING] {}", msg ) << std::endl;
	else
		std::cout << std::format( "[WARNING] {} {}:{}\n\t{}", file, line, col, msg ) << std::endl;
}
