
#pragma once

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitActor : Actor
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitActor& outItem);
    };

    struct PHOENIX_RTS_API UnitActorPtr : ActorPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitActor);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(UnitActor)
}
