
#include "PhoenixRTS/Data/DataEffectSet.h"

using namespace Phoenix::RTS::Data;

bool EffectSet::Read(const LDS::LDSReadObjectArgs& args, EffectSet& outItem)
{
    bool success = Effect::Read(args, outItem);
    return success;
}

EffectSetPtr::EffectSetPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : EffectPtr(path, flags)
{
}

Phoenix::LDS::LDSObjectRefArrayPtr EffectSetPtr::Effects() const
{
    return ObjectRefArray<LDS::LDSObjectRefArrayPtr>("effects");
}
