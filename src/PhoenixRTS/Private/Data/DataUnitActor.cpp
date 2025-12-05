
#include "Data/DataUnitActor.h"

bool Phoenix::RTS::Data::UnitActor::Read(const LDS::LDSReadObjectContext& context, UnitActor& outItem)
{
    bool success = true;

    UnitActorPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Asset.TryGetValue(context, outItem.Asset) && success;
    success = dataPtr.Tint.TryGetValue(context, outItem.Tint) && success;

    return success;
}

Phoenix::RTS::Data::UnitActorPtr::UnitActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Asset(Value<FName>("asset"))
    , Tint(Value<Color>("tint"))
{
}
