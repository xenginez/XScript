#pragma once

#include "type.h"

#include <chrono>

namespace x
{
	class scheduler
	{
    public:
        typedef std::chrono::steady_clock::time_point time_point;
        typedef std::chrono::steady_clock::duration duration;

    public:
        scheduler();
        ~scheduler();

    public:
        //void submit_at( work w, const time_point & tp );
        //void submit_after( work w, const duration & dura );
	};
}
