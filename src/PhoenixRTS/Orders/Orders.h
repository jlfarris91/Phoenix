
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
        Acquire = 4
    };

    struct PHOENIX_RTS_API Order
    {
        EOrderFlags Flags = EOrderFlags::None;
        uint32 Kind;
        FName OrderId = FName::None;
        uint8 OrderIndex = 0;
        ECS::EntityId TargetEntity = ECS::EntityId::Invalid;
        Vec2 TargetLocation;

        bool operator==(const Order& other) const;
        bool operator!=(const Order& other) const;
    };
}
