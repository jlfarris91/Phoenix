
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataEffect.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API EffectSet : Effect
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, EffectSet& outItem);
    };

    struct PHOENIX_RTS_API EffectSetPtr : EffectPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(EffectSet);

        LDS::LDSObjectRefArrayPtr Effects() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(EffectSet)
}
