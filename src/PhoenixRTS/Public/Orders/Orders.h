
#pragma once

#include "Actions.h"
#include "Platform.h"

namespace Phoenix::RTS
{
    enum class EOrderFlags : uint32
    {
        None = 0,
    };

    struct Order
    {
        uint32 AbilityId = Index<uint32>::None;
        EOrderFlags Flags = EOrderFlags::None;
        Action Action;

        bool operator==(const Order& other) const;
        bool operator!=(const Order& other) const;
    };
}
