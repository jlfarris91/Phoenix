
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataCooldown.h"
#include "Phoenix.Sim.RTS/Data/DataEffect.h"
#include "Phoenix.Sim.RTS/Data/DataTargetFilter.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EResponseTarget : uint8
    {
        None,
        Source,
        Target
    };

    struct PHOENIX_RTS_API Response
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Response& outItem);
    };

    struct PHOENIX_RTS_API ResponsePtr : LDS::TLDSObjectPtr<Response>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Response);

        LDS::ValuePtr Chance() const;
        LDS::Int32Ptr Priority() const;
        CooldownPtr Cooldown() const;
        LDS::UInt32Ptr Charges() const;
        EffectRefPtr ResponseEffect() const; 
        EffectRefPtr RequiredEffect() const; 
        LDS::TLDSValuePtr<EResponseTarget> Target() const;
        TargetFilterPtr TargetFilter() const;
        ValidatorRefPtr Validator() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Response)
}
