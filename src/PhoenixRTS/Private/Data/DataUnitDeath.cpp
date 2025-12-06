
#include "Data/DataUnitDeath.h"

using namespace Phoenix::RTS::Data;

bool UnitDeath::Read(const LDS::LDSReadObjectArgs& context, UnitDeath& outItem)
{
    bool success = true;
    return success;
}

UnitDeathPtr::UnitDeathPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
