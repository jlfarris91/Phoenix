
#include "Data/DataUnitSupply.h"

using namespace Phoenix::RTS::Data;

bool UnitSupply::Read(const LDS::LDSReadObjectArgs& context, UnitSupply& outItem)
{
    bool success = true;
    return success;
}

UnitSupplyPtr::UnitSupplyPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
