
#include "Data/DataUnitEffects.h"

using namespace Phoenix::RTS::Data;

bool UnitEffects::Read(const LDS::LDSReadObjectContext& context, UnitEffects& outItem)
{
    bool success = true;
    return success;
}

UnitEffectsPtr::UnitEffectsPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
