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
		const x::meta_type * type() const;
		std::string_view to_string() const;
		int compare( x::object * other ) const;

	protected:
		void mark( x::runtime * rt );

	private:
		x::gcstatus_t get_gcstatus() const;
		void set_gcstatus( x::gcstatus_t status );
	};
}