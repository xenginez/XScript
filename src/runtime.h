#pragma once

#include "value.h"
#include "object.h"

namespace x
{
	class runtime : public std::enable_shared_from_this<runtime>
	{
		friend class object;

	private:
		struct tld;
		struct private_p;

	public:
		runtime();
		~runtime();

	public:
		static runtime * this_thread_owner_runtime();

	public:
		void push( const x::value & val );
		x::value pop();

	public:
		x::uint64 pushed();
		void poped( x::uint64 val );

	public:
		void resize_global( x::uint64 size );
		void resize_thread( x::uint64 size );
		x::value get_global( x::uint64 idx );
		x::value get_thread( x::uint64 idx );

	public:
		void attach_root( x::object * root );
		void detach_root( x::object * root );
		void set_trigger_gc_size( x::uint64 size );

	public:
		tld * create_tld();
		bool delete_tld( tld * val );

	public:
		x::object * obj_alloc( x::uint64 size );
		x::coroutine * coro_alloc( x::uint64 size );

	public:
		void dll_call( std::string_view dllname, std::string_view funcname, x::callmode_t mode, x::uint32 args );

	private:
		void add_gray_object( x::object * gray );
		void add_write_barrier( x::object * owner, x::object * val );

	private:
		void insert_thread_runtime( tld * rt );
		void remove_thread_runtime( tld * rt );

	private:
		void gc();
		void gc_marking();
		void gc_tracking();
		void gc_clearing();
		void gc_collect();

	private:
		private_p * _p;
	};
}
