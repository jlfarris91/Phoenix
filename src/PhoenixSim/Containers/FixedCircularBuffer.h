
#pragma once

#include "FixedQueue.h"

namespace Phoenix
{
    template <class T>
    using TFixedCircularBuffer = TQueue<T, true>;
}
