
#include "Phoenix.Sim.RTS/Data/DataUnitActor.h"

bool Phoenix::RTS::Data::UnitActor::Read(const LDS::LDSReadObjectArgs& args, UnitActor& outItem)
{
    bool success = Actor::Read(args, outItem);
    return success;
}

Phoenix::RTS::Data::UnitActorPtr::UnitActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : ActorPtr(path, flags)
{
}
