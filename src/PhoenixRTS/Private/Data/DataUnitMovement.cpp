#include "Data/DataUnitMovement.h"

using namespace Phoenix::RTS::Data;

bool UnitMovement::Read(const LDS::LDSReadObjectArgs& args, UnitMovement& outItem)
{
    bool success = true;
    return success;
}

UnitMovementPtr::UnitMovementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , AccelerationTime(Value<Time>("acceleration_time"))
    , DecelerationTime(Value<Time>("deceleration_time"))
    , Height(Value<Distance>("height"))
    , LockFacing(Value<bool>("lock_facing"))
    , PathingMode(Value<EPathingMode>("pathing_mode"))
    , PushPriorityAny(Value<uint32>("push_priority_any"))
    , PushPriorityAlly(Value<uint32>("push_priority_ally"))
    , PushDisabled(Value<bool>("push_disabled"))
    , SeparationDelay(Value<Time>("separation_delay"))
    , SeparationRadius(Value<Distance>("separation_radius"))
    , SeparationStrength(Value<Phoenix::Value>("separation_strength"))
    , Speed(Value<Phoenix::Speed>("speed"))
    , TurnRateIdle(Value<Time>("turn_rate_idle"))
    , TurnRateMoving(Value<Time>("turn_rate_moving"))
    , TurnRadius(Value<Distance>("turn_radius"))
    , TurnSlowRate(Value<Phoenix::Value>("turn_slow_rate"))
{
}
