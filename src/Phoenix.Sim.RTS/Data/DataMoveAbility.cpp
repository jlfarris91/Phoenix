
#include "PhoenixRTS/Data/DataMoveAbility.h"

using namespace Phoenix::RTS::Data;

bool MoveAbility::Read(const LDS::LDSReadObjectArgs& args, MoveAbility& outItem)
{
    bool success = true;
    return success;
}

MoveAbilityPtr::MoveAbilityPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : AbilityPtr(path, flags)
{
}
