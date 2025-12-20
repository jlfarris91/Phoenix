
#include "PhoenixRTS/Data/DataCooldown.h"

using namespace Phoenix::RTS::Data;

bool Cooldown::Read(const LDS::LDSReadObjectArgs& args, Cooldown& outItem)
{
    bool success = true;
    return success;
}

CooldownPtr::CooldownPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::TLDSValuePtr<ECooldownScope> CooldownPtr::Scope() const
{
    return Value<LDS::TLDSValuePtr<ECooldownScope>>("scope");
}

Phoenix::LDS::TimePtr CooldownPtr::Duration() const
{
    return Value<LDS::TimePtr>("duration");
}

Phoenix::LDS::NamePtr CooldownPtr::Name() const
{
    return Value<LDS::NamePtr>("name");
}
