
#include "Phoenix.Sim.RTS/Data/DataProjectile.h"

using namespace Phoenix::RTS::Data;

bool Projectile::Read(const LDS::LDSReadObjectArgs& args, Projectile& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    ProjectilePtr dataPtr = args.CreatePtr<ProjectilePtr>();
    success = dataPtr.Actor().TryResolveObject(queryContext, outItem.Actor) && success;
    success = dataPtr.Health().TryGetValue(queryContext, outItem.Health) && success;
    success = dataPtr.Impact().TryReadObject(queryContext, outItem.Impact) && success;
    success = dataPtr.Launch().TryReadObject(queryContext, outItem.Launch) && success;
    success = dataPtr.Movement().TryReadObject(queryContext, outItem.Movement) && success;

    return success;
}

ProjectilePtr::ProjectilePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

ProjectileActorRefPtr ProjectilePtr::Actor() const
{
    return ObjectRef<ProjectileActorRefPtr>("actor");
}

Phoenix::LDS::ValuePtr ProjectilePtr::Health() const
{
    return Value<Phoenix::Value>("health");
}

ProjectileImpactPtr ProjectilePtr::Impact() const
{
    return Object<ProjectileImpactPtr>("impact");
}

ProjectileLaunchPtr ProjectilePtr::Launch() const
{
    return Object<ProjectileImpactPtr>("launch");
}

ProjectileMovementPtr ProjectilePtr::Movement() const
{
    return Object<ProjectileImpactPtr>("movement");
}
