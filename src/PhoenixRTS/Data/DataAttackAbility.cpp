
#include "PhoenixRTS/Data/DataAttackAbility.h"

using namespace Phoenix::RTS::Data;

bool AttackAbility::Read(const LDS::LDSReadObjectArgs& args, AttackAbility& outItem)
{
    bool success = true;
    return success;
}

AttackAbilityPtr::AttackAbilityPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : AbilityPtr(path, flags)
{
}
