#include "scheduler.h"

#include <list>
#include <thread>

#include <concurrentqueue.h>

#include "type.h"

namespace
{
    static constexpr x::int64 N = 60;
    static constexpr x::int64 Si = 10;

    struct timer
    {
        std::chrono::system_clock::time_point time;
        std::function<void()> callback;
    };
    struct timewheel
    {
        x::int64 interval = 0;
        std::list<std::size_t> timers;
    };
}

struct x::scheduler::main_executor::private_p
{
    moodycamel::ConcurrentQueue<std::function<void()>> _tasks;
};

x::scheduler::main_executor::main_executor()
    : _p( new private_p )
{

}

x::scheduler::main_executor::~main_executor()
{
    delete _p;
}

void x::scheduler::main_executor::post( const std::function<void()> & callback )
{
    _p->_tasks.enqueue( callback );
}

void x::scheduler::main_executor::run()
{
    std::function<void()> callback;
    if ( _p->_tasks.try_dequeue( callback ) )
        callback();
}

void x::scheduler::main_executor::shutdown()
{
    std::function<void()> callback;
    while ( _p->_tasks.try_dequeue( callback ) )
        callback();
}

struct x::scheduler::time_executor::private_p
{
    x::uint64 _slot = 0;
    time_point _now = {};
    std::jthread _thread;
    std::vector<timer> _timers;
    std::array<timewheel, N> _timewheels;
    moodycamel::ConcurrentQueue<timer> _queue;
    moodycamel::ConcurrentQueue<std::size_t> _freeidx;
};

x::scheduler::time_executor::time_executor()
    : _p( new private_p )
{
    _p->_now = clock::now();
    _p->_thread = std::jthread( [this]( std::stop_token token )
    {
        printf( "timer thread: %d\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ) );

        while ( !token.stop_requested() )
        {
            tick();

            std::this_thread::yield();
        }
    } );
}

x::scheduler::time_executor::~time_executor()
{
    delete _p;
}

void x::scheduler::time_executor::post( time_point time, const std::function<void()> & callback )
{
    _p->_queue.enqueue( { time, callback } );
}

void x::scheduler::time_executor::tick()
{
    timer tm;
    while ( _p->_queue.try_dequeue( tm ) )
    {
        x::int64 timeout = std::chrono::duration_cast<std::chrono::milliseconds>( tm.time - _p->_now ).count();

        if ( timeout < 0 )
        {
            tm.callback();
            continue;
        }

        x::int64 ticks = timeout / Si;

        x::int64 slot = ( _p->_slot + ( ticks % N ) ) % N;

        std::size_t idx = 0;
        if ( !_p->_freeidx.try_dequeue( idx ) )
        {
            idx = _p->_timers.size();
            _p->_timers.push_back( {} );
        }

        _p->_timers[idx] = std::move( tm );
        _p->_timewheels[slot].timers.push_back( idx );
    }

    auto now = clock::now();
    auto tick = std::chrono::duration_cast<std::chrono::milliseconds>( now - _p->_now ).count();

    if ( tick <= 0 )
        return;

    while ( tick > 0 )
    {
        _p->_timewheels[_p->_slot].interval += tick;

        if ( _p->_timewheels[_p->_slot].interval >= Si )
        {
            auto & timers = _p->_timewheels[_p->_slot].timers;
            for ( auto it = timers.begin(); it != timers.end(); )
            {
                auto p = &_p->_timers[*it];

                if ( p->time <= now )
                {
                    if ( p->callback )
                        p->callback();

                    _p->_freeidx.enqueue( *it );

                    it = timers.erase( it );
                }
                else
                {
                    ++it;
                }
            }

            tick = _p->_timewheels[_p->_slot].interval - Si;
            _p->_timewheels[_p->_slot].interval = 0;
            _p->_slot = ( _p->_slot + 1 ) % N;
        }
        else
        {
            break;
        }
    }

    _p->_now = now;
}

void x::scheduler::time_executor::shutdown()
{
    _p->_thread.request_stop();

    if ( _p->_thread.joinable() )
        _p->_thread.join();
}

struct x::scheduler::work_executor::private_p
{
    std::mutex _mutex;
    std::condition_variable _cond;
    std::vector<std::jthread> _threads;
    moodycamel::ConcurrentQueue<std::function<void()>> _tasks;
};

x::scheduler::work_executor::work_executor()
    : _p( new private_p )
{
    for ( size_t i = 0; i < std::thread::hardware_concurrency(); i++ )
    {
        _p->_threads.push_back( std::jthread( [this, i]( std::stop_token token )
        {
            printf( "work%llu thread: %d\n", i, std::bit_cast<unsigned int>( std::this_thread::get_id() ) );

            std::function<void()> callback;
            while ( !token.stop_requested() )
            {
                if ( _p->_tasks.try_dequeue( callback ) )
                {
                    callback();
                }

                std::unique_lock<std::mutex> lock( _p->_mutex );
                _p->_cond.wait( lock );
            }
        } ) );
    }
}

x::scheduler::work_executor::~work_executor()
{
    delete _p;
}

void x::scheduler::work_executor::post( const std::function<void()> & callback )
{
    _p->_tasks.enqueue( callback );
    _p->_cond.notify_one();
}

void x::scheduler::work_executor::shutdown()
{
    for ( auto & it : _p->_threads )
    {
        it.request_stop();
    }

    _p->_cond.notify_all();

    _p->_threads.clear();
}

struct x::scheduler::alone_executor::private_p
{

};

x::scheduler::alone_executor::alone_executor()
    : _p( new private_p )
{
}

x::scheduler::alone_executor::~alone_executor()
{
    delete _p;
}

void x::scheduler::alone_executor::post( const std::function<void()> & callback )
{
    std::jthread( [this, callback]( auto token )
    {
        printf( "alone thread: %d\n", std::bit_cast<unsigned int>( std::this_thread::get_id() ) );

        callback();

    } ).detach();
}

void x::scheduler::alone_executor::shutdown()
{
}

x::scheduler::scheduler()
{

}

x::scheduler::~scheduler()
{
}

x::scheduler * x::scheduler::instance()
{
    static x::scheduler sc;
    return &sc;
}

void x::scheduler::init()
{
    _main = std::make_unique<main_executor>();
    _timer = std::make_unique<time_executor>();
    _works = std::make_unique<work_executor>();
    _alones = std::make_unique<alone_executor>();
}

bool x::scheduler::run()
{
    if ( _main )
    {
        _main->run();
        return true;
    }

    return false;
}

void x::scheduler::shutdown()
{
    std::thread( [this]()
    {
        _main->shutdown();
        _timer->shutdown();
        _works->shutdown();
        _alones->shutdown();

        _main = nullptr;
        _timer = nullptr;
        _works = nullptr;
        _alones = nullptr;
    } ).detach();
}

x::executor_awaiter x::scheduler::transfer_main()
{
    return { [this]( auto handle ) mutable
    {
        _main->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::transfer_work()
{
    return {
        [this]( auto handle ) mutable
    {
        _works->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::transfer_alone()
{
    return { [this]( auto handle ) mutable
    {
        _alones->post( [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::sleep_for( std::chrono::system_clock::duration duration )
{
    return { [this, duration = std::move( duration )]( auto handle ) mutable
    {
        _timer->post( std::chrono::system_clock::now() + duration, [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}

x::executor_awaiter x::scheduler::sleep_until( std::chrono::system_clock::time_point time_point )
{
    return { [this, time_point = std::move( time_point )]( auto handle ) mutable
    {
        _timer->post( time_point, [handle = std::move( handle )]()
        {
            handle();
        } );
    } };
}
