#include <fstream>
#include <iostream>
#include <filesystem>

#include <json.hpp>
#include <grammar.h>
#include <visitor.h>

int main()
{
	auto path = ( std::filesystem::current_path() / ".." / "test.xs" ).lexically_normal();

	std::ifstream ifs( path );
	if ( ifs.is_open() )
	{
		x::ast_tree_printer_visitor().print( std::cout, x::grammar().parse( ifs, path.string() ) );
	}

	return 0;
}