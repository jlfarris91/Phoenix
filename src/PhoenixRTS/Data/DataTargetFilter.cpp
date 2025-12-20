
#include "PhoenixRTS/Data/DataTargetFilter.h"

using namespace Phoenix::RTS::Data;

bool TargetFilter::Read(const LDS::LDSReadObjectArgs& args, TargetFilter& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    TargetFilterPtr dataPtr = args.CreatePtr<TargetFilterPtr>();
    success = dataPtr.Tags().TryReadObject(lds, outItem.TagFilter) && success;

    return success;
}

TargetFilterPtr::TargetFilterPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

TagFilterPtr TargetFilterPtr::Tags() const
{
    return Object<TagFilterPtr>("tags");
}
