
#include "PhoenixRTS/Data/DataMinimapIcon.h"

using namespace Phoenix::RTS::Data;

bool MinimapIcon::Read(const LDS::LDSReadObjectArgs& args, MinimapIcon& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    MinimapIconPtr dataPtr = args.CreatePtr<MinimapIconPtr>();
    success = dataPtr.Asset().TryGetValue(queryContext, outItem.Asset) && success;
    success = dataPtr.Flags().TryGetValue(queryContext, outItem.Flags) && success;
    success = dataPtr.Icon().TryReadObject(queryContext, outItem.Icon) && success;
    success = dataPtr.PixelSize().TryGetValue(queryContext, outItem.PixelSize) && success;

    return success;
}

MinimapIconPtr::MinimapIconPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::NamePtr MinimapIconPtr::Asset() const
{
    return Value<FName>("asset");
}

Phoenix::LDS::TLDSEnumFlagsPtr<EMinimapIconFlags> MinimapIconPtr::Flags() const
{
    return EnumFlags<EMinimapIconFlags>("flags");
}

IconPtr MinimapIconPtr::Icon() const
{
    return Object<IconPtr>("icon");
}

Phoenix::LDS::UInt32Ptr MinimapIconPtr::PixelSize() const
{
    return Value<uint32>("pixel_size");
}
