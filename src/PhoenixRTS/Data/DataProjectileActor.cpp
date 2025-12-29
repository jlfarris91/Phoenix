
#include "PhoenixRTS/Data/DataProjectileActor.h"

bool Phoenix::RTS::Data::ProjectileActor::Read(const LDS::LDSReadObjectArgs& args, ProjectileActor& outItem)
{
    bool success = Actor::Read(args, outItem);
    return success;
}

Phoenix::RTS::Data::ProjectileActorPtr::ProjectileActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : ActorPtr(path, flags)
{
}
