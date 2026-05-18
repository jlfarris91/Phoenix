
#include "Phoenix.Sim.RTS/Data/DataActor.h"

bool Phoenix::RTS::Data::Actor::Read(const LDS::LDSReadObjectArgs& args, Actor& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    ActorPtr dataPtr = args.CreatePtr<ActorPtr>();
    success = dataPtr.Asset().TryGetValue(queryContext, outItem.Asset) && success;
    success = dataPtr.Scale().TryGetValue(queryContext, outItem.Scale) && success;
    success = dataPtr.Tint().TryGetValue(queryContext, outItem.Tint) && success;

    return success;
}

Phoenix::RTS::Data::ActorPtr::ActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::NamePtr Phoenix::RTS::Data::ActorPtr::Asset() const
{
    return Value<FName>("asset");
}

Phoenix::LDS::ValuePtr Phoenix::RTS::Data::ActorPtr::Scale() const
{
    return Value<Phoenix::Value>("scale");
}

Phoenix::LDS::ColorPtr Phoenix::RTS::Data::ActorPtr::Tint() const
{
    return Value<Color>("tint");
}
