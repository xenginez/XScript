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
		virtual bool is_array() const;
		virtual bool is_callable() const;
		virtual bool is_coroutine() const;

	public:
		virtual x::uint64 size() const;
		virtual x::uint64 hashcode() const;
		virtual x::string to_string() const;
		virtual void copy( x::object * obj );
		virtual void from_string( x::string str );
		virtual const x::meta_type * type() const;
		virtual int compare( x::object * other ) const;

	protected:
		virtual void mark( x::runtime * rt );

	private:
		x::gcstatus_t get_gcstatus() const;
		void set_gcstatus( x::gcstatus_t status );
	};

	class coroutine : public object
	{
		friend class runtime;

	public:
		bool is_coroutine() const override;

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