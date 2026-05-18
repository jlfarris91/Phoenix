
#include "Phoenix.Sim.RTS/Data/DataTargetFilter.h"

using namespace Phoenix::RTS::Data;

bool TargetFilter::Read(const LDS::LDSReadObjectArgs& args, TargetFilter& outItem)
{
    bool success = true;

    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    TargetFilterPtr dataPtr = args.CreatePtr<TargetFilterPtr>();
    success = dataPtr.Tags().TryReadObject(lds, outItem.TagFilter) && success;
    success = dataPtr.Alliance().TryGetValue(lds, outItem.Alliance) && success;

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

Phoenix::LDS::TLDSEnumFlagsPtr<ETargetFilterAllianceFlags> TargetFilterPtr::Alliance() const
{
    return EnumFlags<ETargetFilterAllianceFlags>("alliance");
}
