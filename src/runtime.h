#pragma once

#include "value.h"

namespace x
{
	class runtime
	{
	private:
		struct private_p;

	private:
		runtime();
		~runtime();

	public:
		static x::context_ptr context();
		static void reset( const x::context_ptr & ctx );

	public:
		static void push( x::value val );
		static x::value pop();

	public:
		static x::value thread( uint64_t idx );
		static x::value global( uint64_t idx );

    public:
        static void * alloc( uint64_t size );

	public:
		static void exec_ir( uint64_t code );
		static void exec_jit( uint64_t code );
		static void exec_aot( uint64_t code );
		static void exec_extern( x::static_string_view lib, x::static_string_view proc );

	private:
		static private_p * private_ptr();
	};
}