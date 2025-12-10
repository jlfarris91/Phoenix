
#pragma once

#include "Actions.h"
#include "EntityId.h"
#include "Platform.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::RTS
{
    enum class EOrderType : uint16
    {
        Order = 1,
        Queue = 2,
        Preempt = 4,
        Acquire = 8,
        Smart = 16
    };

    enum class EOrderPriorityType : uint8
    {
        Order,
        Queue
    };

    struct Order
    {
        FName AbilityId = FName::None;
        uint8 CommandIndex = 0;
        ECS::EntityId Target = ECS::EntityId::Invalid;
        Vec2 Location;
        uint32 Flags = 0;

        bool operator==(const Order& other) const;
        bool operator!=(const Order& other) const;
    };
}
