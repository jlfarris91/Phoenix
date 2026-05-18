
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataExpression.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Validator
    {
        TExpressionPtr<bool> Expression;
        FName Error;

        static bool Read(const LDS::LDSReadObjectArgs& args, Validator& outItem);
    };

    struct PHOENIX_RTS_API ValidatorPtr : LDS::TLDSObjectPtr<Validator>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Validator)

        LDS::TLDSObjectRefPtr<TExpression<bool>> Expression() const;
        LDS::TLDSValuePtr<FName> Error() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Validator)
}
