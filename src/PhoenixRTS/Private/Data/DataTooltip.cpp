
#include "Data/DataTooltip.h"

using namespace Phoenix::RTS::Data;

bool TooltipItem::Read(const LDS::LDSReadObjectContext& context, TooltipItem& outItem)
{
    bool success = true;
    TooltipItemPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Label.TryGetValue(context, outItem.Label) && success;
    success = dataPtr.Value.TryGetValue(context, outItem.Value) && success;
    success = dataPtr.Validator.TryResolveObject(context, outItem.Validator) && success;
    return success;
}

TooltipItemPtr::TooltipItemPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Label(TLDSObjectPtr::Value<FName>("label"))
    , Value(TLDSObjectPtr::Value<FName>("value"))
    , Validator(ObjectRef<Data::Validator>("validator"))
{
}

bool Tooltip::Read(const LDS::LDSReadObjectContext& context, Tooltip& outItem)
{
    bool success = true;

    TooltipPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Title.TryGetValue(context, outItem.Title) && success;
    success = dataPtr.SubTitle.TryGetValue(context, outItem.SubTitle) && success;
    success = dataPtr.Body.TryGetValue(context, outItem.Body) && success;

    outItem.Items.Reset();
    dataPtr.Items.ReadObjects(context, outItem.Items);

    return success;
}

TooltipPtr::TooltipPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Title(Value<FName>("title"))
    , SubTitle(Value<FName>("sub_title"))
    , Body(Value<FName>("body"))
    , Items(ObjectArray<TooltipItem>("items"))
{
}