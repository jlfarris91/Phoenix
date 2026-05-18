
#include "Phoenix.Sim.RTS/Data/DataProjectileMovement.h"

using namespace Phoenix::RTS::Data;

bool ProjectileMovement::Read(const LDS::LDSReadObjectArgs& args, ProjectileMovement& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    ProjectileMovementPtr dataPtr = args.CreatePtr<ProjectileMovementPtr>();
    success = dataPtr.AccelerationTime().TryGetValue(queryContext, outItem.AccelerationTime) && success;
    success = dataPtr.Flags().TryGetValue(queryContext, outItem.Flags) && success;
    success = dataPtr.MaxSpeed().TryGetValue(queryContext, outItem.MaxSpeed) && success;
    success = dataPtr.Radius().TryGetValue(queryContext, outItem.Radius) && success;

    return success;
}

ProjectileMovementPtr::ProjectileMovementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TimePtr ProjectileMovementPtr::AccelerationTime() const
{
    return Value<Time>("acceleration_time");
}

Phoenix::LDS::TLDSEnumFlagsPtr<EProjectileMovementFlags> ProjectileMovementPtr::Flags() const
{
    return EnumFlags<EProjectileMovementFlags>("flags");
}

Phoenix::LDS::SpeedPtr ProjectileMovementPtr::MaxSpeed() const
{
    return Value<Speed>("max_speed");
}

Phoenix::LDS::DistancePtr ProjectileMovementPtr::Radius() const
{
    return Value<Distance>("radius");
}
