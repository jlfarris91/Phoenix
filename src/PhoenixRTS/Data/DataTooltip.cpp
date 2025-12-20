
#include "PhoenixRTS/Data/DataTooltip.h"

using namespace Phoenix::RTS::Data;

bool Tooltip::Read(const LDS::LDSReadObjectArgs& args, Tooltip& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    TooltipPtr dataPtr = args.CreatePtr<TooltipPtr>();
    success = dataPtr.Title.TryGetValue(queryContext, outItem.Title) && success;
    success = dataPtr.SubTitle.TryGetValue(queryContext, outItem.SubTitle) && success;
    success = dataPtr.Body.TryGetValue(queryContext, outItem.Body) && success;

    outItem.Items.Reset();
    dataPtr.Items.ReadObjects(queryContext, outItem.Items);

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