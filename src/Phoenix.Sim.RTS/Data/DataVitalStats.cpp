
#include "Phoenix.Sim.RTS/Data/DataVitalStats.h"

using namespace Phoenix::RTS::Data;

bool VitalStats::Read(const LDS::LDSReadObjectArgs& args, VitalStats& outItem)
{
    const LDS::ILDSQueryContext& lds = args.GetQueryContext();

    bool success = true;

    VitalStatsPtr dataPtr = args.CreatePtr<VitalStatsPtr>();
    success = dataPtr.Starting().TryGetValue(lds, outItem.Starting) && success;
    success = dataPtr.Max().TryGetValue(lds, outItem.Max) && success;
    success = dataPtr.Regen().TryGetValue(lds, outItem.Regen) && success;

    return success;
}

VitalStatsPtr::VitalStatsPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::ValuePtr VitalStatsPtr::Starting() const
{
    return Value<Phoenix::Value>("starting");
}

Phoenix::LDS::ValuePtr VitalStatsPtr::Max() const
{
    return Value<Phoenix::Value>("max");
}

Phoenix::LDS::ValuePtr VitalStatsPtr::Regen() const
{
    return Value<Phoenix::Value>("regen");
}
