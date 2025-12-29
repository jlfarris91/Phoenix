
#include "PhoenixRTS/Data/DataIcon.h"

using namespace Phoenix::RTS::Data;

bool Icon::Read(const LDS::LDSReadObjectArgs& args, Icon& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    IconPtr dataPtr = args.CreatePtr<IconPtr>();
    success = dataPtr.Asset().TryGetValue(queryContext, outItem.Asset) && success;
    success = dataPtr.DisplayName().TryGetValue(queryContext, outItem.DisplayName) && success;
    success = dataPtr.Tooltip().TryReadObject(queryContext, outItem.Tooltip) && success;

    return success;
}

IconPtr::IconPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::NamePtr IconPtr::Asset() const
{
    return Value<FName>("asset");
}

Phoenix::LDS::NamePtr IconPtr::DisplayName() const
{
    return Value<FName>("display_name");
}

TooltipPtr IconPtr::Tooltip() const
{
    return ObjectRef<Data::Tooltip>("tooltip");
}
