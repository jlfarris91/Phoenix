
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API ProjectileActor : Actor
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, ProjectileActor& outItem);
    };

    struct PHOENIX_RTS_API ProjectileActorPtr : ActorPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(ProjectileActor);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(ProjectileActor)
}
