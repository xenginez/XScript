#pragma once

#include <coroutine>
#include <functional>

#include "type.h"

namespace x
{
    class schedule_awaitable
    {
        friend class x::scheduler;

    public:
        using coroutine_handle_type = std::coroutine_handle<>;

    public:
        bool await_ready();

        void await_resume();

    public:
        template <typename Handle> coroutine_handle_type await_suspend( Handle h )
        {
            return x::scheduler::transfer( h );
        }
    };

    class scheduler
    {
        friend class schedule_awaitable;

    public:
        using coroutine_handle_type = std::coroutine_handle<>;

    private:
        struct private_p;

    private:
        scheduler();
        ~scheduler();

    private:
        static scheduler * instance();

    public:
        static void co_spawn( coroutine_handle_type coro );

        static coroutine_handle_type transfer( coroutine_handle_type handle );

    public:
        template<typename F> static schedule_awaitable async_post_main( F && callback )
        {

        }

        template<typename F> static schedule_awaitable async_post_pool( F && callback )
        {

        }

        template<typename F> static schedule_awaitable async_post_alone( F && callback )
        {

        }

        static schedule_awaitable async_sleep( std::chrono::system_clock::time_point && time );

        template<typename Clock = std::chrono::system_clock> static schedule_awaitable async_sleep( Clock::time_point && time )
        {
            return async_sleep( std::invoke( std::chrono::clock_time_conversion<std::chrono::system_clock, Clock>(), std::move( time ) ) );
        }

        template<typename Clock = std::chrono::system_clock> static schedule_awaitable async_sleep( Clock::duration && duration )
        {
            return async_sleep( Clock::now() + duration );
        }

    private:
        static schedule_awaitable _async_post( const std::function<void()> & callback );

    private:
        private_p * _p;
    };
}