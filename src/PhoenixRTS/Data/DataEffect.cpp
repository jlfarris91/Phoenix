
#include "PhoenixRTS/Data/DataEffect.h"

using namespace Phoenix::RTS::Data;

bool Effect::Read(const LDS::LDSReadObjectArgs& args, Effect& outItem)
{
    bool success = true;
    return success;
}

EffectPtr::EffectPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

EffectTargetPtr EffectPtr::Target() const
{
    return Object<EffectTargetPtr>("target");
}

TargetFilterPtr EffectPtr::TargetFilter() const
{
    return Object<TargetFilterPtr>("target_filter");
}

ValidatorRefPtr EffectPtr::Validator() const
{
    return ObjectRef<ValidatorRefPtr>("validator");
}
