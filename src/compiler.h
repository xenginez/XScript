#pragma once

#include "type.h"

namespace x
{
	class compiler : public std::enable_shared_from_this<compiler>
	{
	public:
		virtual ~compiler() = default;
	};

	class ir_compiler : public compiler
	{

	};

	class jit_compiler : public compiler
	{

	};

	class aot_compiler : public compiler
	{

	};
}