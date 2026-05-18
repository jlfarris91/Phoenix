
#include "Phoenix.Sim.RTS/Data/DataFaction.h"

using namespace Phoenix::RTS::Data;

bool Faction::Read(const LDS::LDSReadObjectArgs& args, Faction& outItem)
{
    bool success = true;
    return success;
}

FactionPtr::FactionPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}
