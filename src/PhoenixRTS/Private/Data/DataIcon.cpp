
#include "Data/DataIcon.h"

using namespace Phoenix::RTS::Data;

bool Icon::Read(const LDS::LDSReadObjectContext& context, Icon& outItem)
{
    bool success = true;

    IconPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.Asset.TryGetValue(context, outItem.Asset) && success;
    success = dataPtr.DisplayName.TryGetValue(context, outItem.DisplayName) && success;
    success = dataPtr.Tooltip.TryReadObject(context, outItem.Tooltip) && success;

    return success;
}

IconPtr::IconPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Asset(Value<FName>("asset"))
    , DisplayName(Value<FName>("display_name"))
    , Tooltip(ObjectRef<Data::Tooltip>("tooltip"))
{
}
