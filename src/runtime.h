#pragma once

#include "value.h"
#include "object.h"

namespace x
{
	class runtime : public std::enable_shared_from_this<runtime>
	{
	public:
		void push( const x::value & val );
		x::value & top();
		x::value pop();

	public:
		x::uint64 pushed( std::string_view name );
		void poped( x::uint64 val );

	public:
		x::value & thread( x::uint64 idx );
		x::value & global( x::uint64 idx );
	};
}