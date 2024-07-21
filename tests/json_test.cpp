
#include <json.hpp>

int main()
{
	x::json json;

	json.load( R"(
{
	"null":null,
	"bool1" : true,
	"bool2" : false,
	"int" : -124,
	"uint" : 124,
	"float" : 123.456,
	"string" : "aaavdsgfdgdf",
	"array" : [
		"dsafdg",
		"dsafdg",
		"dsafdg"
	],
	"object" : {
		"o_null":null,
		"o_bool1" : true,
		"o_bool2" : false,
		"o_int" : -124,
		"o_uint" : 124,
		"o_float" : 123.456,
		"o_string" : "aaavdsgfdgdf",
	}
}
)" );
	std::cout << json.save( false ) << std::endl;

	json.clear();

	const char * str = "fdsgkfdhgufid";
	//std::string str = "fdsgkfdhgufid";
	//std::string_view str = "fdsgkfdhgufid";

	json["null"] = nullptr;
	json["bool1"] = true;
	json["bool2"] = false;
	json["int"] = -123;
	json["uint"] = 123;
	json["float"] = 123.456;
	json["string"] = str;

	std::vector<x::json> arr;
	{
		arr.push_back( nullptr );
		arr.push_back( true );
		arr.push_back( false );
		arr.push_back( -123 );
		arr.push_back( 123 );
		arr.push_back( 123.456 );
		arr.push_back( str );
	}
	json.insert( "array", arr );

	std::map<std::string_view, x::json> obj;
	{
		obj.insert( { "null", nullptr } );
		obj.insert( { "bool1", true } );
		obj.insert( { "bool2", false } );
		obj.insert( { "int", -123 } );
		obj.insert( { "uint", 123 } );
		obj.insert( { "float", 123.456 } );
		obj.insert( { "string", str } );
	}
	json.insert( "object", obj );

	std::cout << json.save( false ) << std::endl;

	return 0;
}