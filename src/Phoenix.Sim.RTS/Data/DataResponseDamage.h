
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataResponse.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API ResponseDamage : Response
    {
        Value Amount;
        Value AmountScalar = 1.0;
        Value Clamp;
        Value ClampRemainderScalar;
        static bool Read(const LDS::LDSReadObjectArgs& args, ResponseDamage& outItem);
    };

    struct PHOENIX_RTS_API ResponseDamagePtr : ResponsePtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(ResponseDamage);

        LDS::ValuePtr Amount() const;
        LDS::ValuePtr AmountScalar() const;
        LDS::ValuePtr Clamp() const;
        LDS::ValuePtr ClampRemainderScalar() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(ResponseDamage)
}
