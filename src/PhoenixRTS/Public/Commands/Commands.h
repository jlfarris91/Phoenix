
#pragma once

#include "Actions.h"
#include "Platform.h"
#include "Name.h"
#include "EntityId.h"
#include "FixedPoint/FixedVector.h"

namespace Phoenix::RTS
{
    enum class ECommandType : uint8
    {
        Invalid,
        Order,
        Queued
    };

    bool FromVerb(const FName& verb, ECommandType& outType);
    FName ToVerb(ECommandType type);

    enum class ESmartCommandType : uint8
    {
        Invalid,
        Command,
        Queued,
        Rally
    };

    struct Command
    {
        Command() = default;
        Command(const Action& action);

        bool IsValid() const;

        uint32 Sender = 0;
        ECommandType Type = ECommandType::Order;
        FName AbilityId;
        uint8 CommandIndex = 0;
        ECS::EntityId TargetEntity;
        Vec2 TargetLocation;

        operator Phoenix::Action() const;
    };
}
