#include "scheduler.h"

#include <list>
#include <thread>
#include <functional>

#include "concurrent_queue.hpp"

struct x::scheduler::time_executor
{
public:
    using clock = std::chrono::system_clock;
    using duration = clock::duration;
    using time_point = clock::time_point;

public:
    static constexpr x::int64 N = 60;
    static constexpr x::int64 Si = 10;

public:
    struct timer
    {
        time_point time;
        std::function<void()> callback;
    };
    struct timewheel
    {
        x::int64 interval = 0;
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
                run();
            }
        } );
    }

public:
    void post( const std::function<void()> && callback, duration time )
    {
        _queue.enqueue( { clock::now() + time, callback } );
    }
    void post( const std::function<void()> && callback, time_point time )
    {
        _queue.enqueue( { time, callback } );
    }

public:
    void run()
    {
        timer tm;
        while ( _queue.try_dequeue( tm ) )
        {
            x::int64 timeout = std::chrono::duration_cast<std::chrono::milliseconds>( tm.time - _now ).count();

            x::int64 ticks = timeout / Si;

            x::int64 slot = ( _slot + ( ticks % N ) ) % N;

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
        auto tick = std::chrono::duration_cast<std::chrono::milliseconds>( clock::now() - _now ).count();

        while ( tick > 0 )
        {
            _timewheels[_slot].interval += tick;

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

            if ( _timewheels[_slot].interval >= Si )
            {
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
    void shutdown()
    {
        _thread.request_stop();

        if ( _thread.joinable() )
            _thread.join();
    }

private:
    x::uint64 _slot = 0;
    time_point _now = {};
    std::jthread _thread;
    std::vector<timer> _timers;
    x::concurrent_queue<timer> _queue;
    std::array<timewheel, N> _timewheels;
    x::concurrent_queue<std::size_t> _freeidx;
};

struct x::scheduler::private_p
{
    x::scheduler::time_executor time;
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