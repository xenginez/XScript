#pragma once

#include "value.h"
#include "object.h"

namespace x
{
	class runtime : public std::enable_shared_from_this<runtime>
	{
	private:
		struct private_p;

	public:
		runtime();
		~runtime();

	public:
		void push( const x::value & val );
		x::value & top();
		x::value pop();

	public:
		x::uint64 pushed();
		void poped( x::uint64 val );

	public:
		void resize_global( x::uint64 size );
		void resize_thread( x::uint64 size );
		x::value & get_global( x::uint64 idx );
		x::value & get_thread( x::uint64 idx );

	public:
		x::byte * alloc( x::uint64 size );
		void add_root( x::object * root );
		void add_write_barriers( x::object * obj );

	private:
		void gc();
		void gc_marking();
		void gc_tracking();
		void gc_clearing();
		void gc_arrange();

	private:
		private_p * _p;
	};
}
