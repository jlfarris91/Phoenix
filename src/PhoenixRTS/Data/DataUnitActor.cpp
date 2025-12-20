
#include "PhoenixRTS/Data/DataUnitActor.h"

bool Phoenix::RTS::Data::UnitActor::Read(const LDS::LDSReadObjectArgs& args, UnitActor& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    UnitActorPtr dataPtr = args.CreatePtr<UnitActorPtr>();
    success = dataPtr.Asset.TryGetValue(queryContext, outItem.Asset) && success;
    success = dataPtr.Tint.TryGetValue(queryContext, outItem.Tint) && success;

    return success;
}

Phoenix::RTS::Data::UnitActorPtr::UnitActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Asset(Value<FName>("asset"))
    , Tint(Value<Color>("tint"))
{
}
