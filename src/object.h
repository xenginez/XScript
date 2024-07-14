#pragma once

#include "type.h"

namespace x
{
	class object
	{
		friend class runtime;

	public:
		virtual void finalize();

	public:
		virtual bool is_dataobject() const;
		virtual bool is_callobject() const;
		virtual bool is_coroobject() const;

	public:
		virtual x::uint64 size() const;
		virtual x::uint64 hashcode() const;
		virtual x::string to_string() const;
		virtual void from_string( x::string str );
		virtual const x::meta_type * type() const;
		virtual int compare( x::object * other ) const;

	protected:
		virtual void mark( x::runtime * rt );

	private:
		x::gcstatus_t get_gcstatus() const;
		void set_gcstatus( x::gcstatus_t status );
	};

	class array_object : public object
	{

	};

	class callable_object : public object
	{

	};

	class coroutine_object : public object
	{
	public:
		bool done() const;
		bool next() const;
		const x::value & wait() const;
		const x::value & value() const;
		void resume( const x::value & val );
		void except( const x::value & val );

	private:
		x::corostatus_t _status = x::corostatus_t::RESUME;
	};
}