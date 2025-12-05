
#include "Data/DataUnitFlags.h"

using namespace Phoenix::RTS::Data;

bool UnitFlags::Read(const LDS::LDSReadObjectContext& context, UnitFlags& outItem)
{
    bool success = true;
    return success;
}

UnitFlagsPtr::UnitFlagsPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
