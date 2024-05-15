#include "src/context.h"

#include "src/compiler.h"

int main()
{
	x::compiler comp;
	auto module = comp.compile( std::filesystem::current_path() / "script_0.ts" );


	return 0;
}
