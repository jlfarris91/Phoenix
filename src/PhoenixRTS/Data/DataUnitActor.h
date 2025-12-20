
#pragma once

#include "PhoenixSim/Color.h"
#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitActor
    {
        FName Asset;
        Color Tint;

        static bool Read(const LDS::LDSReadObjectArgs& args, UnitActor& outItem);
    };

    struct PHOENIX_RTS_API UnitActorPtr : LDS::TLDSObjectPtr<UnitActor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitActor);
        LDS::NamePtr Asset;
        LDS::TLDSValuePtr<Color> Tint;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(UnitActor)
}
