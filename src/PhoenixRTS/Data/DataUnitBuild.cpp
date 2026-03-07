
#include "PhoenixRTS/Data/DataUnitBuild.h"

using namespace Phoenix::RTS::Data;

bool UnitBuild::Read(const LDS::LDSReadObjectArgs& args, UnitBuild& outItem)
{
    bool success = true;
    return success;
}

UnitBuildPtr::UnitBuildPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
