
#pragma once

#include "Phoenix.Sim/Actions.h"
#include "Phoenix/FixedPoint/FixedVector.h"
#include "Phoenix.Sim.ECS/EntityId.h"
#include "Phoenix/Reflection/Registration.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;
}

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ECommandFlags : uint8
    {
        Invalid = 0,
        Replace = 1,
        Queue = 2,
        Smart = 4
    };

    PHOENIX_RTS_API bool FromVerb(const FName& verb, ECommandFlags& outFlags);
    PHOENIX_RTS_API FName ToVerb(ECommandFlags flags);

    // A command issued by a player to a selection of units.
    struct PHOENIX_RTS_API Command
    {
        PHX_DECLARE_TYPE(Command)

        Command() = default;
        Command(const Action& action);

        bool IsValid() const;

        // An optional id of a player that issued the command.
        uint32 Sender = 0;

        // Flags used to determine how the command will be processed.
        ECommandFlags Flags = ECommandFlags::Replace;

        // An optional contextual identifier for consumers to use for further filtering and customization.
        uint32 Kind = 0;

        // An optional id used to determine which ability will fulfill the command.
        FName CommandId;

        // An identifier used to determine which ability state machine to execute.
        // A command index of 0 is usually the default behavior of an ability.
        uint8 CommandIndex = 0;

        // An optional entity targeted by the command.
        ECS::EntityId TargetEntity;

        // The location targeted by the command.
        // This may or may not be the location of the target entity, if specified, depending on the command and the
        // ability that will fulfill the command.
        Vec2 TargetLocation;

        operator Phoenix::Action() const;
    };

    struct PHOENIX_SIM_API AcquireRequest
    {
        FName Verb;
        
        // An optional contextual identifier for consumers to use for further filtering and customization.
        uint32 Kind = 0;

        // An optional entity targeted by the command.
        ECS::EntityId TargetEntity;

        // The location targeted by the command.
        // This may or may not be the location of the target entity, if specified, depending on the command and the
        // ability that will fulfill the command.
        Vec2 TargetLocation;
    };

    struct PHOENIX_SIM_API AcquireResult
    {
        // An optional id used to determine which ability will fulfill the command.
        FName CommandId;

        // An identifier used to determine which ability state machine to execute.
        // A command index of 0 is usually the default behavior of an ability.
        uint8 CommandIndex = 0;
    };
}

PHX_DEFINE_TYPE(Phoenix::RTS::Command)
{
    registration
        .Field("Sender",            &RTS::Command::Sender)
        .Field("Flags",             &RTS::Command::Flags)
        .Field("Kind",              &RTS::Command::Kind)
        .Field("CommandId",         &RTS::Command::CommandId)
        .Field("CommandIndex",      &RTS::Command::CommandIndex)
        .Field("TargetEntity",      &RTS::Command::TargetEntity)
        .Field("TargetLocation",    &RTS::Command::TargetLocation);
}
