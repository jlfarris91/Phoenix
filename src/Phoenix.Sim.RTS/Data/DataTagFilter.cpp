
#include "Phoenix.Sim.RTS/Data/DataTagFilter.h"

using namespace Phoenix::RTS::Data;

bool TagFilter::Read(const LDS::LDSReadObjectArgs& args, TagFilter& outItem)
{
    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    TagFilterPtr dataPtr = args.CreatePtr<TagFilterPtr>();
    dataPtr.Required().GetResolvedObjects(lds, outItem.Required);
    dataPtr.Excluded().GetResolvedObjects(lds, outItem.Excluded);

    return true;
}

TagFilterPtr::TagFilterPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

TagRefArrayPtr TagFilterPtr::Required() const
{
    return ObjectRefArray<TagRefArrayPtr>("required");
}

TagRefArrayPtr TagFilterPtr::Excluded() const
{
    return ObjectRefArray<TagRefArrayPtr>("excluded");
}
