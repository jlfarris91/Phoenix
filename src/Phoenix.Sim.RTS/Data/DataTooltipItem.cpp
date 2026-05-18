
#include "PhoenixRTS/Data/DataTooltipItem.h"

using namespace Phoenix::RTS::Data;

bool TooltipItem::Read(const LDS::LDSReadObjectArgs& args, TooltipItem& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;
    TooltipItemPtr dataPtr = args.CreatePtr<TooltipItemPtr>();
    success = dataPtr.Label().TryGetValue(queryContext, outItem.Label) && success;
    success = dataPtr.Value().TryGetValue(queryContext, outItem.Value) && success;
    success = dataPtr.Validator().TryResolveObject(queryContext, outItem.Validator) && success;
    return success;
}

TooltipItemPtr::TooltipItemPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> TooltipItemPtr::Label() const
{
    return TLDSObjectPtr::Value<FName>("label");
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> TooltipItemPtr::Value() const
{
    return TLDSObjectPtr::Value<FName>("value");
}

ValidatorRefPtr TooltipItemPtr::Validator() const
{
    return ObjectRef<Data::Validator>("validator");
}
