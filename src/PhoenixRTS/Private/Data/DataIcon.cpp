
#include "Data/DataIcon.h"

using namespace Phoenix::RTS::Data;

bool Icon::Read(const LDS::LDSReadObjectArgs& args, Icon& outItem)
{
    bool success = true;

    LDSScopedObjectQuery scope = (args.GetQueryContext(), args.GetRecordPath());

    IconPtr dataPtr = args.CreatePtr<IconPtr>();
    success = dataPtr.Asset.TryGetValue(outItem.Asset) && success;
    success = dataPtr.DisplayName.TryGetValue(args, outItem.DisplayName) && success;
    success = dataPtr.Tooltip.TryReadObject(args, outItem.Tooltip) && success;

    return success;
}

IconPtr::IconPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Asset(Value<FName>("asset"))
    , DisplayName(Value<FName>("display_name"))
    , Tooltip(ObjectRef<Data::Tooltip>("tooltip"))
{
}
