#pragma once

#include <thread>

#include "Platform.h"

namespace Phoenix
{
    struct SpinBackoff
    {
        uint32 Attempts = 0;
        uint32 PauseAttempts;

        explicit SpinBackoff(uint32 pauseAttempts = 8) : PauseAttempts(pauseAttempts) {}

        void Tick()
        {
            if (Attempts < PauseAttempts)
            {
                const uint32 count = Attempts < 8 ? (1u << Attempts) : 128u;
                for (uint32 i = 0; i < count; ++i)
                    PHX_THREAD_PAUSE();
            }
            else
            {
                std::this_thread::yield();
            }
            ++Attempts;
        }
    };
}
