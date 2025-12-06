
#include "Data/DataUnitVitals.h"

using namespace Phoenix::RTS::Data;

bool UnitVitals::Read(const LDS::LDSReadObjectArgs& context, UnitVitals& outItem)
{
    bool success = true;
    return success;
}

UnitVitalsPtr::UnitVitalsPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
