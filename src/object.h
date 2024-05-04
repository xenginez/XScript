#pragma once

#include "type.h"

namespace x
{
	class object
	{
	public:
		void finalize();
		x::uint64 hashcode() const;
		x::meta_type_ptr type() const;
		std::string_view to_string() const;
		int compare( x::object * other ) const;
	};
}