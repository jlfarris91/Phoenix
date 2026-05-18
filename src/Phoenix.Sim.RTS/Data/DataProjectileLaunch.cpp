
#include "Phoenix.Sim.RTS/Data/DataProjectileLaunch.h"

using namespace Phoenix::RTS::Data;

bool ProjectileLaunch::Read(const LDS::LDSReadObjectArgs& args, ProjectileLaunch& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    ProjectileLaunchPtr dataPtr = args.CreatePtr<ProjectileLaunchPtr>();
    success = dataPtr.FX().TryResolveObject(queryContext, outItem.FX) && success;
    success = dataPtr.Pitch().TryGetValue(queryContext, outItem.Pitch) && success;
    success = dataPtr.PitchRand().TryGetValue(queryContext, outItem.PitchRand) && success;
    success = dataPtr.Roll().TryGetValue(queryContext, outItem.Roll) && success;
    success = dataPtr.RollRand().TryGetValue(queryContext, outItem.RollRand) && success;
    success = dataPtr.Offset().TryGetValue(queryContext, outItem.Offset) && success;
    success = dataPtr.Socket().TryGetValue(queryContext, outItem.Socket) && success;

    return success;
}

ProjectileLaunchPtr::ProjectileLaunchPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

FXActorRefPtr ProjectileLaunchPtr::FX() const
{
    return ObjectRef<FXActorRefPtr>("fx");
}

Phoenix::LDS::Vec3Ptr ProjectileLaunchPtr::Offset() const
{
    return Value<Vec3>("offset");
}

Phoenix::LDS::AnglePtr ProjectileLaunchPtr::Pitch() const
{
    return Value<Angle>("pitch");
}

Phoenix::LDS::AnglePtr ProjectileLaunchPtr::PitchRand() const
{
    return Value<Angle>("pitch_rand");
}

Phoenix::LDS::AnglePtr ProjectileLaunchPtr::Roll() const
{
    return Value<Angle>("roll");
}

Phoenix::LDS::AnglePtr ProjectileLaunchPtr::RollRand() const
{
    return Value<Angle>("roll_rand");
}

Phoenix::LDS::NamePtr ProjectileLaunchPtr::Socket() const
{
    return Value<FName>("socket");
}
