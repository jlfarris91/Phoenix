
#include "PhoenixRTS/Data/DataProjectileActor.h"

bool Phoenix::RTS::Data::ProjectileActor::Read(const LDS::LDSReadObjectArgs& args, ProjectileActor& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    ProjectileActorPtr dataPtr = args.CreatePtr<ProjectileActorPtr>();
    success = dataPtr.Asset.TryGetValue(queryContext, outItem.Asset) && success;
    success = dataPtr.Tint.TryGetValue(queryContext, outItem.Tint) && success;

    return success;
}

Phoenix::RTS::Data::ProjectileActorPtr::ProjectileActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Asset(Value<FName>("asset"))
    , Tint(Value<Color>("tint"))
{
}
