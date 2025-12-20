
#include "PhoenixRTS/Data/DataProjectile.h"

using namespace Phoenix::RTS::Data;

bool Projectile::Read(const LDS::LDSReadObjectArgs& args, Projectile& outItem)
{
    bool success = true;
    return success;
}

ProjectilePtr::ProjectilePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Actor(ObjectRef<ProjectileActorRefPtr>("actor"))
    , Health(Value<Phoenix::Value>("health"))
    , Radius(Value<Distance>("radius"))
{
}
