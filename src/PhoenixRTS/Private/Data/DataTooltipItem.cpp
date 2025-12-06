
#include "Data/DataTooltipItem.h"

using namespace Phoenix::RTS::Data;

bool TooltipItem::Read(const LDS::LDSReadObjectArgs& args, TooltipItem& outItem)
{
    const LDS::ILDSQueryContext& queryContext = *args.GetQueryContext();

    bool success = true;
    TooltipItemPtr dataPtr = args.CreatePtr<TooltipItemPtr>();
    success = dataPtr.Label.TryGetValue(queryContext, outItem.Label) && success;
    success = dataPtr.Value.TryGetValue(queryContext, outItem.Value) && success;
    success = dataPtr.Validator.TryResolveObject(queryContext, outItem.Validator) && success;
    return success;
}

TooltipItemPtr::TooltipItemPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Label(TLDSObjectPtr::Value<FName>("label"))
    , Value(TLDSObjectPtr::Value<FName>("value"))
    , Validator(ObjectRef<Data::Validator>("validator"))
{
}