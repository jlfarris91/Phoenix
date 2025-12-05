
#include "Data/DataUnitPlacement.h"

using namespace Phoenix::RTS::Data;

bool UnitPlacement::Read(const LDS::LDSReadObjectContext& context, UnitPlacement& outItem)
{
    bool success = true;

    UnitPlacementPtr dataPtr(context.Path, context.Flags);
    success = dataPtr.InnerRadius.TryGetValue(context, outItem.InnerRadius) && success;
    success = dataPtr.OuterRadius.TryGetValue(context, outItem.OuterRadius) && success;
    
    return success;
}

UnitPlacementPtr::UnitPlacementPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , InnerRadius(Value<Distance>("inner_radius"))
    , OuterRadius(Value<Distance>("outer_radius"))
{
}
