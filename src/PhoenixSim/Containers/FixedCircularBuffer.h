
#pragma once

#include "FixedQueue.h"

namespace Phoenix
{
    template <class T, size_t N>
    using TFixedCircularBuffer = TFixedQueue<T, N, true>;
}
