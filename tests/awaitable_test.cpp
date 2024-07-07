#include "../src/xlib.h"
#include "../src/coroutine.h"

#include <thread>
#include <iostream>

x::awaitable<int64> seq_index()
{
	int64 i = 0;

	for ( size_t i = 0; i < 10; i++ )
	{
		i = co_await x::awaiter<int64>( []( auto result )
		{
			std::thread( [result]()
			{
				//std::cout << "thread: " << std::this_thread::get_id() << std::endl;

				std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

				result->resume( x_time_now() );

			} ).detach();
		} );

		co_yield i;
	}

	co_return i;
}

void awaitable_test()
{
	std::cout << "-----begin  " << x_time_to_string( x_time_now(), "yyyy-MM-dd hh:mm:ss.zzz" ) << "  begin-----" << std::endl;

	auto idx = seq_index();

	while ( !idx.done() )
	{
		if ( idx.next() )
			std::cout << "-------end  " << x_time_to_string( idx.value(), "yyyy-MM-dd hh:mm:ss.zzz" ) << "  end-------" << std::endl;
	}

	std::cout << "-------end  " << x_time_to_string( idx.value(), "yyyy-MM-dd hh:mm:ss.zzz" ) << "  end-------" << std::endl;
}

int main()
{
	awaitable_test();

	system( "pause" );

	return 0;
}