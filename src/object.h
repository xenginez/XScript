#pragma once

#include "exception.h"

namespace x
{
	class object
	{
		friend class runtime;

	public:
		virtual void construct();
		virtual void finalize();

	public:
		virtual x::uint64 hashcode() const;
		virtual const x::meta_type * type() const;

	public:
		virtual void copyto( x::object * obj );
		virtual int compare( x::object * other ) const;

	public:
		virtual const x::value & get( x::string name ) const;
		virtual void set( x::string name, const x::value & val );
		virtual void invoke( const std::vector<x::value> & args, x::value & result );

	protected:
		virtual void mark( x::runtime * rt );

	private:
		x::gcstatus_t _gcstatus = x::gcstatus_t::WHITE;
	};

	class coroutine : public object
	{
	public:
		bool done() const;
		bool next() const;
		bool error() const;
		bool empty() const;
		x::corostatus_t status() const;

	public:
		const x::value & wait() const;
		const x::value & value() const;
		const x::runtime_exception & exception() const;

	public:
		void reset();
		void resume( const x::value & val );
		void except( const x::runtime_exception & val );

	private:
		int _step = 0;
		std::array<x::byte, 64> _data = {};
		x::corostatus_t _status = x::corostatus_t::EMPTY;
	};
}