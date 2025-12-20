
#include "PhoenixRTS/Data/DataEffectDamage.h"

using namespace Phoenix::RTS::Data;

bool EffectDamage::Read(const LDS::LDSReadObjectArgs& args, EffectDamage& outItem)
{
    bool success = Effect::Read(args, outItem);
    return success;
}

EffectDamagePtr::EffectDamagePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : EffectPtr(path, flags)
{
}

Phoenix::LDS::ValuePtr EffectDamagePtr::Amount() const
{
    return Value<LDS::ValuePtr>("amount");
}

TagRefArrayPtr EffectDamagePtr::DamageTags() const
{
    return ObjectRefArray<TagRefArrayPtr>("damage_tags");
}

TagBonusRefArrayPtr EffectDamagePtr::TagBonuses() const
{
    return ObjectRefArray<TagBonusRefArrayPtr>("tag_bonuses");
}
