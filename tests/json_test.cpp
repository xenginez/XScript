#include <fstream>
#include <filesystem>

#include <json.hpp>

int main()
{
	x::json json, json2;

	std::fstream fs( std::filesystem::current_path() / "xlserver.json" );
	if ( fs.is_open() )
	{
		json = x::json::load( fs );

		auto requests = json["requests"].to_object();
		for ( const auto & it : requests )
		{
			auto s = it.first;
			size_t p = s.find( '/' );
			while ( p != std::string::npos )
			{
				s.replace( p, 1, "_" );
				p = s.find( '/' );
			}
			std::cout << "_methods[\"" << it.first << "\"] = &xlserver::" << s << ";" << std::endl;
		}
		auto notifications = json["notifications"].to_object();
		for ( const auto & it : notifications )
		{
			auto s = it.first;
			size_t p = s.find( '/' );
			while ( p != std::string::npos )
			{
				s.replace( p, 1, "_" );
				p = s.find( '/' );
			}
			std::cout << "_methods[\"" << it.first << "\"] = &xlserver::" << s << ";" << std::endl;
		}
	}

	return 0;
}