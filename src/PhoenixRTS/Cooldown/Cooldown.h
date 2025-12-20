#pragma once

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix
{
    struct FName;
    class World;
}

namespace Phoenix::ECS
{
    struct EntityId;
}

namespace Phoenix::RTS::Data
{
    struct Cooldown;
}

namespace Phoenix::RTS
{
    struct PHOENIX_RTS_API Cooldown
    {
        static bool IsCooldownActive(
            const World& world,
            const ECS::EntityId& entityId,
            const FName& cooldownId,
            const Data::Cooldown& cooldown);
    };
}
