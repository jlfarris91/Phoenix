
#include "Data/DataUnitPlacement.h"

using namespace Phoenix::RTS::Data;

bool UnitPlacement::Read(const LDS::LDSReadObjectArgs& args, UnitPlacement& outItem)
{
    const LDS::ILDSQueryContext& queryContext = *args.GetQueryContext();

    bool success = true;

    UnitPlacementPtr dataPtr = args.CreatePtr<UnitPlacementPtr>();
    success = dataPtr.InnerRadius.TryGetValue(queryContext, outItem.InnerRadius) && success;
    success = dataPtr.OuterRadius.TryGetValue(queryContext, outItem.OuterRadius) && success;
    
    return success;
}

UnitPlacementPtr::UnitPlacementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , InnerRadius(Value<Distance>("inner_radius"))
    , OuterRadius(Value<Distance>("outer_radius"))
{
}
