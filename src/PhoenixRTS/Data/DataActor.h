
#pragma once

#include "PhoenixSim/Color.h"
#include "PhoenixSim/LDS/LDSObjectModel.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Actor
    {
        FName Asset;
        Color Tint;

        static bool Read(const LDS::LDSReadObjectArgs& args, Actor& outItem);
    };

    struct PHOENIX_RTS_API ActorPtr : LDS::TLDSObjectPtr<Actor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Actor)

        LDS::NamePtr Asset() const;
        LDS::ColorPtr Tint() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Actor)
}
