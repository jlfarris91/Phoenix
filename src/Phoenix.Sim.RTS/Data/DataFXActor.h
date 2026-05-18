
#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API FXActor : Actor
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, FXActor& outItem);
    };

    struct PHOENIX_RTS_API FXActorPtr : ActorPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(FXActor);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(FXActor)
}
