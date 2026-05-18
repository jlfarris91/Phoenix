
#include "Phoenix.Sim.RTS/Data/DataResponse.h"

using namespace Phoenix::RTS::Data;

bool Response::Read(const LDS::LDSReadObjectArgs& args, Response& outItem)
{
    bool success = true;
    return success;
}

ResponsePtr::ResponsePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::ValuePtr ResponsePtr::Chance() const
{
    return Value<LDS::ValuePtr>("chance");
}

Phoenix::LDS::Int32Ptr ResponsePtr::Priority() const
{
    return Value<LDS::Int32Ptr>("priority");
}

CooldownPtr ResponsePtr::Cooldown() const
{
    return Object<CooldownPtr>("cooldown");
}

Phoenix::LDS::UInt32Ptr ResponsePtr::Charges() const
{
    return Value<LDS::UInt32Ptr>("charges");
}

EffectRefPtr ResponsePtr::ResponseEffect() const
{
    return ObjectRef<EffectRefPtr>("response_effect");
}

EffectRefPtr ResponsePtr::RequiredEffect() const
{
    return ObjectRef<EffectRefPtr>("required_effect");
}

Phoenix::LDS::TLDSValuePtr<EResponseTarget> ResponsePtr::Target() const
{
    return Value<EResponseTarget>("target");
}

TargetFilterPtr ResponsePtr::TargetFilter() const
{
    return Object<TargetFilterPtr>("target_filter");
}

ValidatorRefPtr ResponsePtr::Validator() const
{
    return ObjectRef<ValidatorRefPtr>("validator");
}
