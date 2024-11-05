#include <fstream>
#include <iostream>
#include <filesystem>

#include <logger.h>
#include <symbols.h>
#include <visitor.h>
#include <grammar.h>
#include <semantic.h>
#include <optimize.h>

int main()
{
	auto path = ( std::filesystem::current_path() / "test.xs" ).lexically_normal();

	std::ifstream ifs( path );
	if ( ifs.is_open() )
	{
		auto logger = std::make_shared<x::logger>();
		auto symbols = std::make_shared<x::symbols>();

		auto unit = x::grammar().parse( ifs, path.string() );
		
		x::symbol_scan_visitor().scan( logger, symbols, unit );
		x::semantics_analysis_visitor().analysis( logger, symbols, unit );

		x::astree_print_visitor().print( std::cout, unit );
		std::cout << "------------------optimize------------------" << std::endl;
		x::code_optimize_visitor().optimize( logger, symbols, unit );
		x::astree_print_visitor().print( std::cout, unit );
	}

	return 0;
}