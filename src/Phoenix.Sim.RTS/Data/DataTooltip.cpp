
#include "Phoenix.Sim.RTS/Data/DataTooltip.h"

using namespace Phoenix::RTS::Data;

bool Tooltip::Read(const LDS::LDSReadObjectArgs& args, Tooltip& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    TooltipPtr dataPtr = args.CreatePtr<TooltipPtr>();
    success = dataPtr.Title().TryGetValue(queryContext, outItem.Title) && success;
    success = dataPtr.SubTitle().TryGetValue(queryContext, outItem.SubTitle) && success;
    success = dataPtr.Body().TryGetValue(queryContext, outItem.Body) && success;

    outItem.Items.clear();
    dataPtr.Items().ReadObjects(queryContext, outItem.Items);

    return success;
}

TooltipPtr::TooltipPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> TooltipPtr::Title() const
{
    return Value<FName>("title");
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> TooltipPtr::SubTitle() const
{
    return Value<FName>("sub_title");
}

Phoenix::LDS::TLDSValuePtr<Phoenix::FName> TooltipPtr::Body() const
{
    return Value<FName>("body");
}

Phoenix::LDS::TLDSObjectArrayPtr<TooltipItem> TooltipPtr::Items() const
{
    return ObjectArray<TooltipItem>("items");
}
