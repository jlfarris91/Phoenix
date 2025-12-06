
#include "Data/DataFogVisibility.h"

using namespace Phoenix::RTS::Data;

bool FogVisibility::Read(const LDS::LDSReadObjectArgs& context, FogVisibility& outItem)
{
    bool success = true;
    return success;
}

FogVisibilityPtr::FogVisibilityPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
