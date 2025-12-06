
#include "Data/DataUnitCargo.h"

using namespace Phoenix::RTS::Data;

bool UnitCargo::Read(const LDS::LDSReadObjectArgs& context, UnitCargo& outItem)
{
    bool success = true;
    return success;
}

UnitCargoPtr::UnitCargoPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
