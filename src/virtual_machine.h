#pragma once

#include "type.h"

namespace x
{
	class virtual_machine : public std::enable_shared_from_this<virtual_machine>
	{
	public:
		virtual_machine();
		~virtual_machine();

	public:
		int exec( const x::runtime_ptr & rt, const x::context_ptr & ctx );
	};
}