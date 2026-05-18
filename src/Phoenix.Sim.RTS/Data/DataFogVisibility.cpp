
#include "Phoenix.Sim.RTS/Data/DataFogVisibility.h"

using namespace Phoenix::RTS::Data;

bool FogVisibility::Read(const LDS::LDSReadObjectArgs& args, FogVisibility& outItem)
{
    bool success = true;
    return success;
}

FogVisibilityPtr::FogVisibilityPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
