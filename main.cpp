#include "src/runtime.h"
#include "src/context.h"
#include "src/compiler.h"
#include "src/virtual_machine.h"

int main()
{
	auto  rt = std::make_shared<x::runtime>();
	auto  ctx = std::make_shared<x::context>();
	auto  vm = std::make_shared<x::virtual_machine>();
	auto  comp = std::make_shared<x::module_compiler>();

	if ( comp->compile( std::filesystem::current_path() / "script_0.xs" ) )
	{
		if ( ctx->load( comp->module() ) )
		{
			return vm->exec( rt, ctx );
		}
	}

	return 0;
}
