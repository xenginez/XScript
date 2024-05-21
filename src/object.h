#pragma once

#include "type.h"

namespace x
{
	class object
	{
		friend class runtime;

	public:
		void finalize();

	public:
		x::uint64 size() const;
		x::uint64 hashcode() const;
		x::meta_type_ptr type() const;
		std::string_view to_string() const;
		int compare( x::object * other ) const;

	protected:
		void mark( x::runtime * rt );

	protected:
		x::gcstatus_t status = x::gcstatus_t::WHITE;
	};
}