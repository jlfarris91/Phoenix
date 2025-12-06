
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "DataExpression.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Validator
    {
        TExpressionPtr<bool> Expression;
        FName Error;

        static bool Read(const LDS::LDSReadObjectArgs& context, Validator& outItem);
    };

    struct PHOENIX_RTS_API ValidatorPtr : LDS::TLDSObjectPtr<Validator>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Validator)

        LDS::TLDSObjectRefPtr<TExpression<bool>> Expression;
        LDS::TLDSValuePtr<FName> Error;
    };
}
