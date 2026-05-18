
#include "Phoenix.Sim.RTS/Data/DataFXActor.h"

bool Phoenix::RTS::Data::FXActor::Read(const LDS::LDSReadObjectArgs& args, FXActor& outItem)
{
    bool success = Actor::Read(args, outItem);
    return success;
}

Phoenix::RTS::Data::FXActorPtr::FXActorPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : ActorPtr(path, flags)
{
}
