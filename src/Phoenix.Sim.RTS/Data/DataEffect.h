
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataEffectTarget.h"
#include "Phoenix.Sim.RTS/Data/DataTargetFilter.h"
#include "Phoenix.Sim.RTS/Data/DataValidator.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Effect
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Effect& outItem);
    };

    struct PHOENIX_RTS_API EffectPtr : LDS::TLDSObjectPtr<Effect>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Effect);

        EffectTargetPtr Target() const;
        TargetFilterPtr TargetFilter() const;
        ValidatorRefPtr Validator() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Effect)
}
