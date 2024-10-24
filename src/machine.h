#pragma once

#include "type.h"

namespace x
{
	class machine : public std::enable_shared_from_this<machine>
	{
		struct private_p;

	public:
		machine();
		~machine();

	public:
		int exec( const x::runtime_ptr & rt, const x::context_ptr & ctx );

	private:
		private_p * _p;
	};
}