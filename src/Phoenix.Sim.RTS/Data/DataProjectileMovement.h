
#pragma once

#include "Phoenix.Sim.RTS/Data/DataFXActor.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EProjectileMovementFlags : uint8
    {
        None = 0,
        LockFacing = 1,
        LockHeightOffset = 2
    };
    
    struct PHOENIX_RTS_API ProjectileMovement
    {
        Time AccelerationTime;
        EProjectileMovementFlags Flags = EProjectileMovementFlags::None;
        Speed MaxSpeed;
        Distance Radius;

        static bool Read(const LDS::LDSReadObjectArgs& args, ProjectileMovement& outItem);
    };

    struct PHOENIX_RTS_API ProjectileMovementPtr : LDS::TLDSObjectPtr<ProjectileMovement>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(ProjectileMovement)

        LDS::TimePtr AccelerationTime() const;
        LDS::TLDSEnumFlagsPtr<EProjectileMovementFlags> Flags() const;
        LDS::SpeedPtr MaxSpeed() const;
        LDS::DistancePtr Radius() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(ProjectileMovement)
}
