#pragma once

#include <chrono>
#include <variant>
#include <coroutine>
#include <functional>
#include <type_traits>

namespace x
{
	template<typename T, typename... Args> struct has_await_ready
	{
		template<typename U> static constexpr auto check( const void * ) -> decltype( std::declval<U>().await_ready( std::declval<Args>()... ), std::true_type() );
		template<typename U> static constexpr std::false_type check( ... );

		static constexpr bool value = decltype( check<T>( nullptr ) )::value;
	};
	template<typename T, typename... Args> struct has_await_suspend
	{
		template<typename U> static constexpr auto check( const void * ) -> decltype( std::declval<U>().await_suspend( std::declval<Args>()... ), std::true_type() );
		template<typename U> static constexpr std::false_type check( ... );

		static constexpr bool value = decltype( check<T>( nullptr ) )::value;
	};
	template<typename T, typename... Args> struct has_await_resume
	{
		template<typename U> static constexpr auto check( const void * ) -> decltype( std::declval<U>().await_resume( std::declval<Args>()... ), std::true_type() );
		template<typename U> static constexpr std::false_type check( ... );

		static constexpr bool value = decltype( check<T>( nullptr ) )::value;
	};
	template<typename T> static constexpr bool is_awaiter_v = has_await_ready<T>::value && has_await_suspend<T, std::coroutine_handle<>>::value && has_await_resume<T>::value;

	class empty_awaitable
	{
	public:
		struct promise_type
		{
			std::suspend_never initial_suspend() noexcept
			{
				return {};
			}

			std::suspend_never final_suspend() noexcept
			{
				return {};
			}

			void return_void() noexcept
			{
			}

			void unhandled_exception()
			{
			}

			empty_awaitable get_return_object() noexcept
			{
				return empty_awaitable();
			}
		};
	};

	template <typename T> class final_awaitable : public std::suspend_always
	{
	public:
		template<typename PromiseType> std::coroutine_handle<> await_suspend( std::coroutine_handle<PromiseType> h ) noexcept
		{
			if ( h.promise()._handle )
			{
				return h.promise()._handle;
			}
			return std::noop_coroutine();
		}
	};

	class awaitable_promise_base
	{
	public:
		auto initial_suspend()
		{
			return std::suspend_always{};
		}

		template <typename A, std::enable_if_t<is_awaiter_v<A>, int> = 0> auto await_transform( A awaiter ) const
		{
			return awaiter;
		}

	public:
		std::coroutine_handle<> _handle;
	};

	template <typename T> class awaitable
	{
	public:
		struct promise_type : public awaitable_promise_base
		{
			awaitable<T> get_return_object()
			{
				return awaitable<T>{ std::coroutine_handle<promise_type<T>>::from_promise( *this ) };
			}

			auto final_suspend() noexcept
			{
				return final_awaitable<T>{};
			}

			template <typename V> void return_value( V && val ) noexcept
			{
				_value.template emplace<T>( std::forward<V>( val ) );
			}

			void unhandled_exception() noexcept
			{
				_value.template emplace<std::exception_ptr>( std::current_exception() );
			}

			std::variant<std::exception_ptr, T> _value{ nullptr };
		};

	public:
		explicit awaitable( std::coroutine_handle<promise_type> h )
			: _current_handle( h )
		{
		}

		~awaitable()
		{
			if ( _current_handle && _current_handle.done() )
			{
				_current_handle.destroy();
			}
		}

		awaitable( awaitable && t ) noexcept
			: _current_handle( t._current_handle )
		{
			t._current_handle = nullptr;
		}

		awaitable & operator=( awaitable && t ) noexcept
		{
			if ( &t != this )
			{
				if ( _current_handle )
				{
					_current_handle.destroy();
				}
				_current_handle = t._current_handle;
				t._current_handle = nullptr;
			}
			return *this;
		}

		awaitable( const awaitable & ) = delete;

		awaitable & operator=( const awaitable & ) = delete;

	public:
		T operator()()
		{
			return get();
		}

	public:
		T get()
		{
			return std::move( _current_handle.promise()._value );
		}

		void detach()
		{
			auto launch_coro = []( awaitable<T> lazy ) -> empty_awaitable
			{
				co_await lazy;
			};

			(void)launch_coro( std::move( *this ) );
		}

	public:
		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		T await_resume()
		{
			auto ret = std::move( _current_handle.promise()._value );
			if ( std::holds_alternative<std::exception_ptr>( ret ) )
			{
				std::rethrow_exception( std::get<std::exception_ptr>( ret ) );
			}

			_current_handle.destroy();
			_current_handle = nullptr;

			return std::get<T>( ret );
		}

		template <typename PromiseType> auto await_suspend( std::coroutine_handle<PromiseType> handle )
		{
			_current_handle.promise()._handle = handle;

			if constexpr ( std::is_base_of_v<awaitable_promise_base, PromiseType> )
			{
				_current_handle.promise().local_ = handle.promise().local_;
			}

			return _current_handle;
		}

	public:
		std::coroutine_handle<promise_type> _current_handle;
	};

	template <> class awaitable<void>
	{
	public:
		struct promise_type : public awaitable_promise_base
		{
			awaitable<void> get_return_object()
			{
				return awaitable<void>{ std::coroutine_handle<promise_type>::from_promise( *this ) };
			}

			auto final_suspend() noexcept
			{
				return final_awaitable<void>{};
			}

			void return_void()
			{
			}

			void unhandled_exception() noexcept
			{
				_exception = std::current_exception();
			}

			std::exception_ptr _exception{ nullptr };
		};

	public:
		explicit awaitable( std::coroutine_handle<promise_type> h )
			: _current_handle( h )
		{
		}

		awaitable( awaitable && t ) noexcept
			: _current_handle( t._current_handle )
		{
			t._current_handle = nullptr;
		}

		awaitable & operator=( awaitable && t ) noexcept
		{
			if ( &t != this )
			{
				if ( _current_handle )
				{
					_current_handle.destroy();
				}
				_current_handle = t._current_handle;
				t._current_handle = nullptr;
			}
			return *this;
		}

		awaitable( const awaitable & ) = delete;

		awaitable & operator=( const awaitable & ) = delete;

		~awaitable()
		{
			if ( _current_handle && _current_handle.done() )
			{
				_current_handle.destroy();
			}
		}

	public:
		void operator()()
		{
			get();
		}

	public:
		void get()
		{
			(void)_current_handle.promise();
		}

		void detach()
		{
			auto launch_coro = []( awaitable<void> lazy ) -> empty_awaitable
			{
				co_await lazy;
			};

			(void)launch_coro( std::move( *this ) );
		}

	public:
		constexpr bool await_ready() const noexcept
		{
			return false;
		}

		void await_resume()
		{
			auto exception = _current_handle.promise()._exception;
			if ( exception )
			{
				std::rethrow_exception( exception );
			}

			_current_handle.destroy();
			_current_handle = nullptr;
		}

		template <typename PromiseType> auto await_suspend( std::coroutine_handle<PromiseType> handle )
		{
			_current_handle.promise()._handle = handle;

			if constexpr ( std::is_base_of_v<awaitable_promise_base, PromiseType> )
			{
				_current_handle.promise().local_ = handle.promise().local_;
			}

			return _current_handle;
		}

		std::coroutine_handle<promise_type> _current_handle;
	};

	class executor_awaiter
	{
	public:
		executor_awaiter( const std::function<void( std::coroutine_handle<> )> & callback )
			: _callback( std::move( callback ) )
		{
		}

	public:
		constexpr bool await_ready() noexcept
		{
			return false;
		}

		auto await_suspend( std::coroutine_handle<> handle )
		{
			_callback( handle );
		}

		void await_resume() noexcept
		{
		}

	private:
		std::function<void( std::coroutine_handle<> )> _callback;
	};

    class scheduler
    {
    private:
        struct main_executor
        {
            struct private_p;

        public:
            main_executor();
            ~main_executor();

        public:
            void post( const std::function<void()> & callback );
            void run();
            void shutdown();

        private:
            private_p * _p;
        };
        struct time_executor
        {
            struct private_p;

        public:
            using clock = std::chrono::system_clock;
            using duration = clock::duration;
            using time_point = clock::time_point;

        public:
            time_executor();
            ~time_executor();

        public:
            void post( time_point time, const std::function<void()> & callback );
            void shutdown();

        public:
            void tick();

        private:
            private_p * _p;
        };
        struct work_executor
        {
            struct private_p;

        public:
            work_executor();
            ~work_executor();

        public:
            void post( const std::function<void()> & callback );
            void shutdown();

        private:
            private_p * _p;
        };
        struct alone_executor
        {
            struct private_p;

        public:
            alone_executor();
            ~alone_executor();

        public:
            void post( const std::function<void()> & callback );
            void shutdown();

        private:
            private_p * _p;
        };

    private:
        scheduler();
        ~scheduler();

    public:
        static scheduler * instance();

    public:
        void init();
        bool run();
        void shutdown();

    public:
        x::executor_awaiter transfer_main();
        x::executor_awaiter transfer_work();
        x::executor_awaiter transfer_alone();
        x::executor_awaiter sleep_for( std::chrono::system_clock::duration duration );
        x::executor_awaiter sleep_until( std::chrono::system_clock::time_point time_point );

    private:
        std::unique_ptr<main_executor> _main;
        std::unique_ptr<time_executor> _timer;
        std::unique_ptr<work_executor> _works;
        std::unique_ptr<alone_executor> _alones;
    };
}
