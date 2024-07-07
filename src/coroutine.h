#pragma once

#include "value.h"

#include <mutex>
#include <coroutine>
#include <functional>

namespace x
{
    template<typename T> class awaiter;
    template<typename T> class awaitable;
    template<typename T> class await_result;

    template<typename T> class await_result
    {
        friend class awaiter<T>;

    public:
        using tuple_type = std::tuple<T, std::coroutine_handle<>>;

    public:
        await_result()
            : _tuple( std::make_shared<tuple_type>() )
        {
        }
        await_result( const await_result & val )
            : _tuple( val._tuple )
        {
        }
        await_result & operator =( const await_result & val )
        {
            _tuple = val._tuple;
            return *this;
        }

    public:
        T value() const
        {
            return std::get<T>( *_tuple );
        }
        void resume( const T & val )
        {
            std::get<T>( *_tuple ) = val;
            std::get<std::coroutine_handle<>>( *_tuple ).resume();
        }

    private:
        void set_handle( std::coroutine_handle<> handle )
        {
            std::get<std::coroutine_handle<>>( *_tuple ) = handle;
        }

    private:
        std::shared_ptr<tuple_type> _tuple;
    };
    template<> class await_result<void>
    {
        friend class awaiter<void>;

    public:
        await_result()
            : _handle( nullptr )
        {
        }
        await_result( const await_result & val )
            : _handle( val._handle )
        {
        }
        await_result & operator =( const await_result & val )
        {
            _handle = val._handle;
            return *this;
        }

    public:
        void resume()
        {
            _handle.resume();
        }

    private:
        void set_handle( std::coroutine_handle<> handle )
        {
            _handle = handle;
        }

    private:
        std::coroutine_handle<> _handle;
    };

    template<typename T> class awaiter
    {
    public:
        using value_type = T;
        using result_type = await_result<T>;

    public:
        using callback_type = std::function<void( result_type * )>;

    public:
        awaiter( const callback_type & callback )
            : _callback( callback )
        {

        }

    public:
        bool await_ready() const
        {
            return false;
        }
        void await_suspend( std::coroutine_handle<> handle )
        {
            _result.set_handle( handle );

            _callback( &_result );
        }
        value_type await_resume() const noexcept
        {
            return _result.value();
        }

    private:
        result_type _result;
        callback_type _callback;
    };
    template<> class awaiter<void>
    {
    public:
        using result_type = await_result<void>;

    public:
        using callback_type = std::function<void( result_type * )>;

    public:
        awaiter( const callback_type & callback )
            : _callback( callback )
        {

        }

    public:
        bool await_ready() const
        {
            return false;
        }
        void await_suspend( std::coroutine_handle<> handle )
        {
            _result.set_handle( handle );

            _callback( &_result );
        }
        void await_resume() const noexcept
        {
        }

    private:
        result_type _result;
        callback_type _callback;
    };

    template<typename T> class awaitable
    {
    public:
        using value_type = T;

    public:
        enum class promise_state
        {
            NONE,
            RESUME,
            RUNNING,
            READY,
        };

    public:
        class promise_type
        {
        public:
            x::awaitable<value_type> get_return_object()
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
                _exception = std::current_exception();
                _state = promise_state::READY;
            }

        public:
            void return_value( value_type value )
            {
                _value = value;
                _state = promise_state::READY;
            }
            std::suspend_always yield_value( value_type value )
            {
                _value = value;
                _state = promise_state::READY;
                return {};
            }

        public:
            promise_state state() const
            {
                return _state;
            }
            value_type value()
            {
                _state = promise_state::RESUME;

                if ( _exception )
                    std::rethrow_exception( _exception );

                return _value;
            }

        private:
            value_type _value = {};
            std::exception_ptr _exception = nullptr;
            promise_state _state = promise_state::NONE;
        };

    public:
        awaitable( std::coroutine_handle<promise_type> handle )
            : _handle( handle )
        {
        }
        awaitable( awaitable && val ) noexcept
            : _handle( std::exchange( val._handle, {} ) )
        {
        }
        awaitable & operator=( nullptr_t ) noexcept
        {
            _handle = nullptr;
            return *this;
        }
        awaitable( const awaitable & ) = delete;
        awaitable & operator=( const awaitable & ) = delete;
        ~awaitable()
        {
            destroy();
        }

    public:
        operator bool() const noexcept
        {
            return _handle != nullptr;
        }

    public:
        bool done() const
        {
            return _handle.done();
        }
        bool next() const
        {
            switch ( _handle.promise().state() )
            {
            case promise_state::NONE:
                _handle.resume();
                return false;
            case promise_state::RESUME:
                return false;
            case promise_state::RUNNING:
                return false;
            case promise_state::READY:
                return true;
            default:
                break;
            }
            return false;
        }

    public:
        value_type wait() const
        {
            while ( !_handle.done() )
            {
                next();
            }
            return _handle.promise().value();
        }
        value_type value() const
        {
            return _handle.promise().value();
        }
        void destroy()
        {
            if ( _handle )
                _handle.destroy();

            _handle = nullptr;
        }

    public:
        void * address()
        {
            return _handle.address();
        }
        static awaitable from_address( void * addr )
        {
            return { std::coroutine_handle<promise_type>::from_address( addr ) };
        }

    private:
        std::coroutine_handle<promise_type> _handle;
    };
    template<> class awaitable<void>
    {
    public:
        class promise_type
        {
        public:
            x::awaitable<void> get_return_object()
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
                _exception = std::current_exception();
            }

        public:
            void return_void()
            {
                _ready = true;
            }
            std::suspend_always yield_value()
            {
                _ready = true;
                return {};
            }

        public:
            bool is_ready() const
            {
                return _ready;
            }
            void value()
            {
                if ( _exception )
                    std::rethrow_exception( _exception );

                _ready = false;
            }

        private:
            bool _ready = false;
            std::exception_ptr _exception;
        };

    public:
        awaitable( std::coroutine_handle<promise_type> handle )
            : _handle( handle )
        {
        }
        awaitable( awaitable && val ) noexcept
            : _handle( std::exchange( val._handle, {} ) )
        {
        }
        awaitable & operator=( nullptr_t ) noexcept
        {
            _handle = nullptr;
            return *this;
        }
        awaitable( const awaitable & ) = delete;
        awaitable & operator=( const awaitable & ) = delete;
        ~awaitable()
        {
            destroy();
        }

    public:
        operator bool() const noexcept
        {
            return _handle != nullptr;
        }

    public:
        bool done() const
        {
            return _handle.done();
        }
        bool next() const
        {
            return _handle.promise().is_ready();
        }

    public:
        void wait() const
        {
            while ( !_handle.done() )
            {
                _handle.resume();
                _handle.promise().value();
            }
        }
        void value() const
        {
            _handle.promise().value();
        }
        void destroy()
        {
            if ( _handle )
                _handle.destroy();

            _handle = nullptr;
        }

    public:
        void * address()
        {
            return _handle.address();
        }
        static awaitable from_address( void * addr )
        {
            return { std::coroutine_handle<promise_type>::from_address( addr ) };
        }

    private:
        std::coroutine_handle<promise_type> _handle;
    };

    template<typename U, typename F> static x::awaitable<U> to_awaitable( F && callback )
    {
        if constexpr ( std::is_void_v<U> )
        {
            co_await awaiter<U>( callback );
            co_return;
        }
        else
        {
            auto result = co_await awaiter<U>( callback );
            co_return result;
        }
    }

    /*
    class scheduler
    {
    private:
        struct private_p;

    public:
        static struct thread_one{} one;
        static struct thread_main{} main;
        static struct thread_pool{} pool;

    private:
        scheduler();
        ~scheduler();

    public:
        static scheduler * instance();

    public:
        awaitable<void> transfer( thread_one );
        awaitable<void> transfer( thread_main );
        awaitable<void> transfer( thread_pool );

    public:
        template<typename F, std::enable_if_t<std::is_void_v<std::invoke_result_t<F>>, int > = 0> awaitable<std::invoke_result_t<F>> post( F && callback );
        template<typename F, std::enable_if_t<!std::is_void_v<std::invoke_result_t<F>>, int > = 0> awaitable<std::invoke_result_t<F>> post( F && callback );

    public:
        void maintick();
        void shutdown();

    private:
        private_p * _p;
    };
    */
}