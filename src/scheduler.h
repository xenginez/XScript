#pragma once

#include "type.h"

namespace x
{
    class scheduler
    {
    private:
        struct private_p;

    private:
        scheduler();
        ~scheduler();

    public:
        struct time_executor;
        struct pool_executor;
        struct main_executor;
        struct alone_executor;

    private:
        static scheduler * instance();

    public:


    private:
        private_p * _p;
    };
}