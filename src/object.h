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
		x::string to_string() const;
		void from_string( x::string str );
		const x::meta_type * type() const;
		int compare( x::object * other ) const;
		void on_event( x::uint32 event, x::int64 val );

	protected:
		void mark( x::runtime * rt );

	private:
		x::gcstatus_t get_gcstatus() const;
		void set_gcstatus( x::gcstatus_t status );
	};

	class callable
	{

	};
}