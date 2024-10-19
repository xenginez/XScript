#include "scheduler.h"

#include <thread>

#include <concurrentqueue.h>

namespace
{
    struct main_executor
    {
    public:
        void post( const std::function<void()> & callback )
        {
            _tasks.enqueue( callback );
        }
        void run()
        {
            std::function<void()> callback;
            if ( _tasks.try_dequeue( callback ) )
                callback();
        }
        void shutdown()
        {
            std::function<void()> callback;
            while ( _tasks.try_dequeue( callback ) )
                callback();
        }

    private:
        moodycamel::ConcurrentQueue<std::function<void()>> _tasks;
    };
    struct time_executor
    {
    public:
        static constexpr std::int64_t N = 60;
        static constexpr std::int64_t Si = 10;

    public:
        using clock = std::chrono::system_clock;
        using duration = clock::duration;
        using time_point = clock::time_point;

    public:
        struct timer
        {
            time_point time;
            std::function<void()> callback;
        };
        struct timewheel
        {
            std::int64_t interval = 0;
            std::list<std::size_t> timers;
        };

    public:
        time_executor()
        {
            _now = clock::now();
            _thread = std::jthread( [this]( std::stop_token token )
            {
                while ( !token.stop_requested() )
                {
                    tick();

                    std::this_thread::yield();
                }
            } );
        }

    public:
        void post( time_point time, const std::function<void()> & callback )
        {
            _timerqueue.enqueue( { time, callback } );
        }
        void shutdown()
        {
            _thread.request_stop();

            if ( _thread.joinable() )
                _thread.join();
        }

    private:
        void tick()
        {
            timer tm;
            while ( _timerqueue.try_dequeue( tm ) )
            {
                std::int64_t timeout = std::chrono::duration_cast<std::chrono::milliseconds>( tm.time - _now ).count();

                if ( timeout <= 0 )
                {
                    tm.callback();
                    continue;
                }

                std::int64_t ticks = timeout / Si;

                std::int64_t slot = ( _slot + ( ticks % N ) ) % N;

                std::size_t idx = 0;
                if ( !_freeidx.try_dequeue( idx ) )
                {
                    idx = _timers.size();
                    _timers.push_back( {} );
                }

                _timers[idx] = std::move( tm );
                _timewheels[slot].timers.push_back( idx );
            }

            auto now = clock::now();
            auto tick = std::chrono::duration_cast<std::chrono::milliseconds>( now - _now ).count();

            if ( tick <= 0 )
                return;

            while ( tick > 0 )
            {
                _timewheels[_slot].interval += tick;

                if ( _timewheels[_slot].interval >= Si )
                {
                    auto & timers = _timewheels[_slot].timers;
                    for ( auto it = timers.begin(); it != timers.end(); )
                    {
                        auto p = &_timers[*it];

                        if ( p->time <= now )
                        {
                            if ( p->callback )
                                p->callback();

                            _freeidx.enqueue( *it );

                            it = timers.erase( it );
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    tick = _timewheels[_slot].interval - Si;
                    _timewheels[_slot].interval = 0;
                    _slot = ( _slot + 1 ) % N;
                }
                else
                {
                    break;
                }
            }

            _now = now;
        }

    private:
        time_point _now = {};
        std::int64_t _slot = 0;
        std::jthread _thread;
        std::vector<timer> _timers;
        std::array<timewheel, N> _timewheels;
        moodycamel::ConcurrentQueue<timer> _timerqueue;
        moodycamel::ConcurrentQueue<std::size_t> _freeidx;
    };
    struct work_executor
    {
    public:
        work_executor()
        {
            for ( size_t i = 0; i < std::thread::hardware_concurrency(); i++ )
            {
                _threads.push_back( std::jthread( [this, i]( std::stop_token token )
                {
                    std::function<void()> callback;
                    while ( !token.stop_requested() )
                    {
                        if ( _tasks.try_dequeue( callback ) )
                        {
                            callback();
                        }

                        std::unique_lock<std::mutex> lock( _mutex );
                        _cond.wait( lock );
                    }
                } ) );
            }
        }

    public:
        void post( const std::function<void()> & callback )
        {
            _tasks.enqueue( callback );
            _cond.notify_one();
        }
        void shutdown()
        {
            for ( auto & it : _threads )
            {
                it.request_stop();
            }

            _cond.notify_all();

            _threads.clear();
        }

    private:
        std::mutex _mutex;
        std::condition_variable _cond;
        std::vector<std::jthread> _threads;
        moodycamel::ConcurrentQueue<std::function<void()>> _tasks;
    };
    struct alone_executor
    {
    public:
        void post( const std::function<void()> & callback )
        {
            std::jthread( [this, callback]( auto token )
            {
                callback();

            } ).detach();
        }
        void shutdown()
        {
        }
    };
}

struct x::scheduler::private_p
{
    std::atomic_bool _shutdown = false;
    std::unique_ptr<main_executor> _main;
    std::unique_ptr<time_executor> _timer;
    std::unique_ptr<work_executor> _works;
    std::unique_ptr<alone_executor> _alones;
};

x::scheduler::scheduler()
    : _p( new private_p )
{

}

x::scheduler::~scheduler()
{
    delete _p;
}

x::scheduler * x::scheduler::instance()
{
    static x::scheduler sc;
    return &sc;
}

void x::scheduler::init()
{
    auto _p = x::scheduler::instance()->_p;

    _p->_shutdown = false;
    _p->_main = std::make_unique<main_executor>();
    _p->_timer = std::make_unique<time_executor>();
    _p->_works = std::make_unique<work_executor>();
    _p->_alones = std::make_unique<alone_executor>();
}

bool x::scheduler::run()
{
    auto _p = x::scheduler::instance()->_p;

    if ( !_p->_shutdown )
    {
        _p->_main->run();
        return true;
    }
    else
    {
        _p->_main->shutdown();
        _p->_timer->shutdown();
        _p->_works->shutdown();
        _p->_alones->shutdown();

        _p->_main = nullptr;
        _p->_timer = nullptr;
        _p->_works = nullptr;
        _p->_alones = nullptr;

        return false;
    }
}

void x::scheduler::shutdown()
{
    x::scheduler::instance()->_p->_shutdown = true;
}

x::executor_awaiter x::scheduler::transfer_main()
{
    return { []( auto handle ) mutable
    {
        x::scheduler::instance()->_p->_main->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::transfer_work()
{
    return {
        []( auto handle ) mutable
    {
        x::scheduler::instance()->_p->_works->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::transfer_alone()
{
    return { []( auto handle ) mutable
    {
        x::scheduler::instance()->_p->_alones->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::sleep_for( std::chrono::system_clock::duration duration )
{
    return { [duration = std::move( duration )]( auto handle ) mutable
    {
        x::scheduler::instance()->_p->_timer->post( std::chrono::system_clock::now() + duration, [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::sleep_until( std::chrono::system_clock::time_point time_point )
{
    return { [time_point = std::move( time_point )]( auto handle ) mutable
    {
        x::scheduler::instance()->_p->_timer->post( time_point, [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}
