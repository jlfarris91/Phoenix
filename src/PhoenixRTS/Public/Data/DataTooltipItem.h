
#pragma once

#include "DataValidator.h"

namespace Phoenix::LDS
{
    struct LDSReadObjectArgs;
}

namespace Phoenix::RTS::Data
{
    struct Validator;

    struct PHOENIX_RTS_API TooltipItem
    {
        FName Label;
        FName Value;
        ValidatorPtr Validator;

        static bool Read(const LDS::LDSReadObjectArgs& args, TooltipItem& outItem);
    };

    struct PHOENIX_RTS_API TooltipItemPtr : LDS::TLDSObjectPtr<TooltipItem>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TooltipItem)

        LDS::TLDSValuePtr<FName> Label;
        LDS::TLDSValuePtr<FName> Value;
        LDS::TLDSObjectRefPtr<Validator> Validator;
    };
}
