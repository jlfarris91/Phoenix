
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitEffects
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitEffects& outItem);
    };

    struct PHOENIX_RTS_API UnitEffectsPtr : LDS::TLDSObjectPtr<UnitEffects>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitEffects);
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitEffects)
}
