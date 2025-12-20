
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/ECS/EntityId.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API EOrderFlags : uint16
    {
        None = 0,
        Replace = 1,
        Queued = 2,
        Smart = 4
    };

    enum class PHOENIX_RTS_API EOrderPriorityType : uint8
    {
        Replace,
        Queue
    };

    struct PHOENIX_RTS_API Order
    {
        FName AbilityId = FName::None;
        uint8 CommandIndex = 0;
        ECS::EntityId TargetEntity = ECS::EntityId::Invalid;
        Vec2 TargetLocation;
        EOrderFlags Flags = EOrderFlags::None;

        bool operator==(const Order& other) const;
        bool operator!=(const Order& other) const;
    };
}
