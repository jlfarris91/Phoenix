
#pragma once

#include "FixedQueue.h"

namespace Phoenix
{
    template <class T, class TStoragePolicy>
    using TFixedCircularBuffer = TQueue<T, TStoragePolicy, true>;
}
