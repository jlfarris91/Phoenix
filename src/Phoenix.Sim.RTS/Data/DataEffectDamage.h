
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataEffect.h"
#include "Phoenix.Sim.RTS/Data/DataTag.h"
#include "Phoenix.Sim.RTS/Data/DataTagBonus.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API EffectDamage : Effect
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, EffectDamage& outItem);
    };

    struct PHOENIX_RTS_API EffectDamagePtr : EffectPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(EffectDamage);

        LDS::ValuePtr Amount() const;
        TagRefArrayPtr DamageTags() const;
        TagBonusRefArrayPtr TagBonuses() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(EffectDamage)
}
