
#include "Phoenix.Sim.RTS/Data/DataFootprint.h"

using namespace Phoenix::RTS::Data;

bool Footprint::Read(const LDS::LDSReadObjectArgs& args, Footprint& outItem)
{
    bool success = true;
    return success;
}

FootprintPtr::FootprintPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::NamePtr FootprintPtr::Asset() const
{
    return Value<FName>("asset");
}

Phoenix::LDS::BoolPtr FootprintPtr::Snap() const
{
    return Value<bool>("snap");
}
