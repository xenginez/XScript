#include <iostream>

#include <zip.h>

int main()
{
	x::zip f;

	f.write( "test/abc.txt", "���ԺͿ����������������ĵ�11111122222222233333333" );
	f.write( "test/123.txt", "11111122222222233333333���ԺͿ����������������ĵ�" );

	std::cout << std::endl;
	for ( auto it : f.namelist() )
	{
		std::cout << it << " --- ";
		f.read( it, std::cout );
		std::cout << std::endl;
	}

	f.reset();

	std::cout << std::endl;
	for ( auto it : f.namelist() )
	{
		std::cout << it << " --- ";
		f.read( it, std::cout );
		std::cout << std::endl;
	}

	{
		std::stringstream ss( std::ios::out | std::ios::binary );
		x::zip::compression( "���ԺͿ����������������ĵ�11111122222222233333333", ss );
		std::stringstream ss2( ss.str(), std::ios::in | std::ios::binary );
		x::zip::decompression( ss2, std::cout );
	}

	return 0;
}