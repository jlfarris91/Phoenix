
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EEffectTargetFlags
    {
        None,
        Entity = 1,
        Location = 2,
        Target = 4,
        Source = 8
    };
    
    struct PHOENIX_RTS_API EffectTarget
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, EffectTarget& outItem);
    };

    struct PHOENIX_RTS_API EffectTargetPtr : LDS::TLDSObjectPtr<EffectTarget>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(EffectTarget);

        LDS::LDSObjectRefPtr EffectParent() const;
        LDS::TLDSEnumFlagsPtr<EEffectTargetFlags> Flags() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(EffectTarget)
}
