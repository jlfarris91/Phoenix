
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EPathingMode
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

        LDS::TimePtr AccelerationTime() const;
        LDS::TimePtr DecelerationTime() const;
        LDS::DistancePtr Height() const;
        LDS::BoolPtr LockFacing() const;
        LDS::TLDSValuePtr<EPathingMode> PathingMode() const;
        LDS::UInt32Ptr PushPriorityAny() const;
        LDS::UInt32Ptr PushPriorityAlly() const;
        LDS::BoolPtr PushDisabled() const;
        LDS::TimePtr SeparationDelay() const;
        LDS::DistancePtr SeparationRadius() const;
        LDS::ValuePtr SeparationStrength() const;
        LDS::SpeedPtr Speed() const;
        LDS::TimePtr TurnRateIdle() const;
        LDS::TimePtr TurnRateMoving() const;
        LDS::DistancePtr TurnRadius() const;
        LDS::ValuePtr TurnSlowRate() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitMovement)
}
