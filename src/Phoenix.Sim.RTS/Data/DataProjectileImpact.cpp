
#include "Phoenix.Sim.RTS/Data/DataProjectileImpact.h"

using namespace Phoenix::RTS::Data;

bool ProjectileImpact::Read(const LDS::LDSReadObjectArgs& args, ProjectileImpact& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    ProjectileImpactPtr dataPtr = args.CreatePtr<ProjectileImpactPtr>();
    success = dataPtr.FX().TryResolveObject(queryContext, outItem.FX) && success;
    success = dataPtr.Offset().TryGetValue(queryContext, outItem.Offset) && success;
    success = dataPtr.Socket().TryGetValue(queryContext, outItem.Socket) && success;

    return success;
}

ProjectileImpactPtr::ProjectileImpactPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

FXActorRefPtr ProjectileImpactPtr::FX() const
{
    return ObjectRef<FXActorRefPtr>("fx");
}

Phoenix::LDS::Vec3Ptr ProjectileImpactPtr::Offset() const
{
    return Value<Vec3>("offset");
}

Phoenix::LDS::NamePtr ProjectileImpactPtr::Socket() const
{
    return Value<FName>("socket");
}
