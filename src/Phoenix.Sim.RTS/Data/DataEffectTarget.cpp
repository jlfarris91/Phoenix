
#include "Phoenix.Sim.RTS/Data/DataEffectTarget.h"

using namespace Phoenix::RTS::Data;

bool EffectTarget::Read(const LDS::LDSReadObjectArgs& args, EffectTarget& outItem)
{
    bool success = true;
    return success;
}

EffectTargetPtr::EffectTargetPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::LDSObjectRefPtr EffectTargetPtr::EffectParent() const
{
    return ObjectRef("effect_parent");
}

Phoenix::LDS::TLDSEnumFlagsPtr<EEffectTargetFlags> EffectTargetPtr::Flags() const
{
    return EnumFlags<EEffectTargetFlags>("flags");
}
