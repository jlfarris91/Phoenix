#include "Phoenix.Sim.RTS/Data/DataUnitMovement.h"

using namespace Phoenix::RTS::Data;

bool UnitMovement::Read(const LDS::LDSReadObjectArgs& args, UnitMovement& outItem)
{
    bool success = true;
    return success;
}

UnitMovementPtr::UnitMovementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TimePtr UnitMovementPtr::AccelerationTime() const
{
    return Value<Time>("acceleration_time");
}

Phoenix::LDS::TimePtr UnitMovementPtr::DecelerationTime() const
{
    return Value<Time>("deceleration_time");
}

Phoenix::LDS::DistancePtr UnitMovementPtr::Height() const
{
    return Value<Distance>("height");
}

Phoenix::LDS::BoolPtr UnitMovementPtr::LockFacing() const
{
    return Value<bool>("lock_facing");
}

Phoenix::LDS::TLDSValuePtr<EPathingMode> UnitMovementPtr::PathingMode() const
{
    return Value<EPathingMode>("pathing_mode");
}

Phoenix::LDS::UInt32Ptr UnitMovementPtr::PushPriorityAny() const
{
    return Value<uint32>("push_priority_any");
}

Phoenix::LDS::UInt32Ptr UnitMovementPtr::PushPriorityAlly() const
{
    return Value<uint32>("push_priority_ally");
}

Phoenix::LDS::BoolPtr UnitMovementPtr::PushDisabled() const
{
    return Value<bool>("push_disabled");
}

Phoenix::LDS::TimePtr UnitMovementPtr::SeparationDelay() const
{
    return Value<Time>("separation_delay");
}

Phoenix::LDS::DistancePtr UnitMovementPtr::SeparationRadius() const
{
    return Value<Distance>("separation_radius");
}

Phoenix::LDS::ValuePtr UnitMovementPtr::SeparationStrength() const
{
    return Value<Phoenix::Value>("separation_strength");
}

Phoenix::LDS::SpeedPtr UnitMovementPtr::Speed() const
{
    return Value<Phoenix::Speed>("speed");
}

Phoenix::LDS::TimePtr UnitMovementPtr::TurnRateIdle() const
{
    return Value<Time>("turn_rate_idle");
}

Phoenix::LDS::TimePtr UnitMovementPtr::TurnRateMoving() const
{
    return Value<Time>("turn_rate_moving");
}

Phoenix::LDS::DistancePtr UnitMovementPtr::TurnRadius() const
{
    return Value<Distance>("turn_radius");
}

Phoenix::LDS::ValuePtr UnitMovementPtr::TurnSlowRate() const
{
    return Value<Phoenix::Value>("turn_slow_rate");
}
