#pragma once

#include "value.h"
#include "object.h"

namespace x
{
	class runtime : public std::enable_shared_from_this<runtime>
	{
		friend class object;

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
		void add_root( x::object * root );
		void set_trigger_gc_size( x::uint64 size );

	public:
		x::object * alloc( x::uint64 size );

	private:
		x::byte * page_alloc( x::uint64 size );
		x::byte * item_alloc( x::uint64 size );
		x::byte * slot_alloc( x::uint64 size );

	private:
		void add_gray( x::object * gray );
		void add_wbarriers( x::object * left, x::object * right );

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
