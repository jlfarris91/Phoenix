
#include "PhoenixRTS/Data/DataAbility.h"

using namespace Phoenix::RTS::Data;

bool Ability::Read(const LDS::LDSReadObjectArgs& args, Ability& outItem)
{
    bool success = true;
    return success;
}

AbilityPtr::AbilityPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

SmartCastPtr AbilityPtr::SmartCast() const
{
    return Object<SmartCastPtr>("smart_cast");
}
