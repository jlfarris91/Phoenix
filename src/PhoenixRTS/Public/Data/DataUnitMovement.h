
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"
#include "DataTooltip.h"

namespace Phoenix::RTS::Data
{
    enum class EPathingMode
    {
        Ground,
        Air,
        Hover
    };

    struct PHOENIX_RTS_API UnitMovement
    {
        Time AccelerationTime;
        Time DecelerationTime;
        Distance Height;
        bool LockFacing;
        EPathingMode PathingMode;
        uint32 PushPriorityAny;
        uint32 PushPriorityAlly;
        bool PushDisabled;
        Time SeparationDelay;
        Distance SeparationRadius;
        Value SeparationStrength;
        Speed Speed;
        Time TurnTimeIdle;
        Time TurnTimeMoving;
        Distance TurnRadius;
        Value TurnSlowRate;

        static bool Read(const LDS::LDSReadObjectArgs& args, UnitMovement& outItem);
    };

    struct PHOENIX_RTS_API UnitMovementPtr : LDS::TLDSObjectPtr<UnitMovement>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitMovement)

        LDS::TLDSValuePtr<Time> AccelerationTime;
        LDS::TLDSValuePtr<Time> DecelerationTime;
        LDS::TLDSValuePtr<Distance> Height;
        LDS::TLDSValuePtr<bool> LockFacing;
        LDS::TLDSValuePtr<EPathingMode> PathingMode;
        LDS::TLDSValuePtr<uint32> PushPriorityAny;
        LDS::TLDSValuePtr<uint32> PushPriorityAlly;
        LDS::TLDSValuePtr<bool> PushDisabled;
        LDS::TLDSValuePtr<Time> SeparationDelay;
        LDS::TLDSValuePtr<Distance> SeparationRadius;
        LDS::TLDSValuePtr<Phoenix::Value> SeparationStrength;
        LDS::TLDSValuePtr<Speed> Speed;
        LDS::TLDSValuePtr<Time> TurnTimeIdle;
        LDS::TLDSValuePtr<Time> TurnTimeMoving;
        LDS::TLDSValuePtr<Distance> TurnRadius;
        LDS::TLDSValuePtr<Phoenix::Value> TurnSlowRate;
    };
}
