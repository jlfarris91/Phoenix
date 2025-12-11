
#include "Data/DataUnitVitals.h"

using namespace Phoenix::RTS::Data;

bool UnitVitals::Read(const LDS::LDSReadObjectArgs& args, UnitVitals& outItem)
{
    bool success = true;
    return success;
}

UnitVitalsPtr::UnitVitalsPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Health(Object<VitalPtr>("health"))
    , Energy(Object<VitalPtr>("energy"))
    , Shield(Object<VitalPtr>("shield"))
{
}
