
#include "Data/DataUnitInfo.h"

using namespace Phoenix::RTS::Data;

bool UnitInfo::Read(const LDS::LDSReadObjectArgs& context, UnitInfo& outItem)
{
    bool success = true;
    return success;
}

UnitInfoPtr::UnitInfoPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
