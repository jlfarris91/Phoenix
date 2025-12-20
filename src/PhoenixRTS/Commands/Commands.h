
#pragma once

#include "PhoenixSim/Actions.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/ECS/EntityId.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ECommandFlags : uint8
    {
        Invalid = 0,
        Replace = 1,
        Queued = 2,
        Smart = 4
    };

    PHOENIX_RTS_API bool FromVerb(const FName& verb, ECommandFlags& outType);
    PHOENIX_RTS_API FName ToVerb(ECommandFlags flags);

    struct PHOENIX_RTS_API Command
    {
        Command() = default;
        Command(const Action& action);

        bool IsValid() const;

        uint32 Sender = 0;
        ECommandFlags Flags = ECommandFlags::Replace;
        FName AbilityId;
        uint8 CommandIndex = 0;
        ECS::EntityId TargetEntity;
        Vec2 TargetLocation;

        operator Phoenix::Action() const;
    };
}
