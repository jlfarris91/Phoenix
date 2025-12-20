
#include "PhoenixRTS/Data/DataUnitPlacement.h"

using namespace Phoenix::RTS::Data;

bool UnitPlacement::Read(const LDS::LDSReadObjectArgs& args, UnitPlacement& outItem)
{
    const LDS::ILDSQueryContext& queryContext = args.GetQueryContext();

    bool success = true;

    UnitPlacementPtr dataPtr = args.CreatePtr<UnitPlacementPtr>();
    success = dataPtr.InnerRadius().TryGetValue(queryContext, outItem.InnerRadius) && success;
    success = dataPtr.OuterRadius().TryGetValue(queryContext, outItem.OuterRadius) && success;
    
    return success;
}

UnitPlacementPtr::UnitPlacementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<Phoenix::TFixed<12>> UnitPlacementPtr::InnerRadius() const
{
    return Value<Distance>("inner_radius");
}

Phoenix::LDS::TLDSValuePtr<Phoenix::TFixed<12>> UnitPlacementPtr::OuterRadius() const
{
    return Value<Distance>("outer_radius");
}
