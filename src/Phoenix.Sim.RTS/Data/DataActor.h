
#pragma once

#include "Phoenix.Sim/Color.h"
#include "Phoenix.Sim.LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Actor
    {
        FName Asset;
        Value Scale = 1.0;
        Color Tint;

        static bool Read(const LDS::LDSReadObjectArgs& args, Actor& outItem);
    };

    struct PHOENIX_RTS_API ActorPtr : LDS::TLDSObjectPtr<Actor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Actor)

        LDS::NamePtr Asset() const;
        LDS::ValuePtr Scale() const;
        LDS::ColorPtr Tint() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Actor)
}
