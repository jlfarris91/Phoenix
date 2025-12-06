
#include "Data/DataUnitVision.h"

using namespace Phoenix::RTS::Data;

bool UnitVision::Read(const LDS::LDSReadObjectArgs& args, UnitVision& outItem)
{
    bool success = true;
    return success;
}

UnitVisionPtr::UnitVisionPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
