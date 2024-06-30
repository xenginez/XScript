#pragma once

#include "value.h"

#include <coroutine>

namespace x
{
	template<typename T> class coroutine
	{
    public:
        bool await_ready()
        {
            return false;
        }
        void await_suspend( std::coroutine_handle<> handle )
        {

        }
        T await_resume()
        {
            return _value;
        }

    private:
        T _value;
	};

    template<> class coroutine<x::value>
    {
    public:
        coroutine();
        ~coroutine();

    public:
        bool await_ready();
        void await_suspend( std::coroutine_handle<> handle );
        x::value await_resume();

    private:
        x::value _value;
    };

    class awaitable
    {
    public:
        class promise_type
        {
            friend class awaitable;

        public:
            awaitable get_return_object()
            {
                return { std::coroutine_handle<promise_type>::from_promise( *this ) };
            }
            std::suspend_always initial_suspend()
            {
                return {};
            }
            std::suspend_always final_suspend() noexcept
            {
                return {};
            }
            void unhandled_exception()
            {
            }

        public:
            void return_void()
            {
            }
            void return_value( x::value value )
            {
                _value = value;
            }
            std::suspend_always yield_value( x::value value )
            {
                _value = value;
                return {};
            }
            std::suspend_always await_transform( x::value value )
            {
                _value = value;
                return {};
            }

        private:
            x::value _value;
            bool _ready = false;
        };

    public:
        awaitable( std::coroutine_handle<promise_type> handle )
            : _handle( handle )
        {
        }
        awaitable( awaitable && val )
            : _handle( std::exchange( val._handle, {} ) )
        { }
        awaitable( const awaitable & ) = delete;
        awaitable & operator=( const awaitable & ) = delete;

    public:
        x::value next()
        {
            _handle.resume();

            return _handle.promise()._value;
        }
        bool has_next() const
        {
            if ( _handle.done() )
                return false;

            if ( !_handle.promise()._ready )
                return false;

            return true;
        }
        void destroy()
        {
            if ( _handle ) _handle.destroy();
        }

    private:
        std::coroutine_handle<promise_type> _handle;
    };
}