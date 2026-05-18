#pragma once

#include <cstdint>

namespace Phoenix
{
    enum class EAppStateFlags : uint8_t
    {
        None = 0,
        Initializing = 1,
        Initialized = 2,
        ShuttingDown = 4,
        ShutDown = 8
    };
}
