#include "src/context.h"

int main()
{
	auto ctx = std::make_shared < x::context >();
	
	ctx->load_stdlib();
	ctx->load_file( std::filesystem::current_path() / "script_0.ts" );

	return 0;
}
