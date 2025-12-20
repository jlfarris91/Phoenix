
#include "PhoenixRTS/Data/DataVital.h"

using namespace Phoenix::RTS::Data;

bool Vital::Read(const LDS::LDSReadObjectArgs& args, Vital& outItem)
{
    bool success = true;
    return success;
}

VitalPtr::VitalPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
    , Starting(Value<Phoenix::Value>("starting"))
    , Max(Value<Phoenix::Value>("max"))
    , Regen(Value<Phoenix::Value>("regen"))
{
}
