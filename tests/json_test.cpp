#include <fstream>
#include <filesystem>

#include <json.hpp>

int main()
{
	x::json json, json2;

	std::fstream fs( std::filesystem::current_path() / "language_server.json" );
	if ( fs.is_open() )
	{
		json.load( fs );

		auto obj = json.to_object();
		for ( const auto & it : obj )
		{
			x::json elem2;
			auto elem = it.second.to_array();
			for ( const auto & it2 : elem )
			{
				if ( it2.contains( "method" ) )
					elem2[it2["method"].to_string()] = it2;
				else if ( it2.contains( "name" ) )
					elem2[it2["name"].to_string()] = it2;
			}
			json2[it.first] = elem2;
		}

		std::fstream fs2( std::filesystem::current_path() / "language_server.json", std::ios::out | std::ios::trunc );

		json2.save( fs2 );
	}

	return 0;
}