
#include "PhoenixRTS/Data/DataVitalStatsPair.h"

using namespace Phoenix::RTS::Data;

bool VitalStatsPair::Read(const LDS::LDSReadObjectArgs& args, VitalStatsPair& outItem)
{
    bool success = true;
    return success;
}

VitalStatsPairPtr::VitalStatsPairPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

VitalRefPtr VitalStatsPairPtr::Vital() const
{
    return ObjectRef<VitalRefPtr>("vital");
}

VitalStatsPtr VitalStatsPairPtr::Stats() const
{
    return Object<VitalStatsPtr>("stats");
}
