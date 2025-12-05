
#pragma once

#include "DataValidator.h"
#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"

namespace Phoenix::LDS
{
    struct LDSReadObjectContext;
}

namespace Phoenix::RTS::Data
{
    struct Validator;

    struct PHOENIX_RTS_API TooltipItem
    {
        FName Label;
        FName Value;
        ValidatorPtr Validator;

        static bool Read(const LDS::LDSReadObjectContext& context, TooltipItem& outItem);
    };

    struct PHOENIX_RTS_API TooltipItemPtr : LDS::TLDSObjectPtr<TooltipItem>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TooltipItem)

        LDS::TLDSValuePtr<FName> Label;
        LDS::TLDSValuePtr<FName> Value;
        LDS::TLDSObjectRefPtr<Validator> Validator;
    };

    struct PHOENIX_RTS_API Tooltip
    {
        FName Title;
        FName SubTitle;
        FName Body;
        TArray2<TooltipItem> Items;

        static bool Read(const LDS::LDSReadObjectContext& context, Tooltip& outItem);
    };

    struct PHOENIX_RTS_API TooltipPtr : LDS::TLDSObjectPtr<Tooltip>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Tooltip)

        LDS::TLDSValuePtr<FName> Title;
        LDS::TLDSValuePtr<FName> SubTitle;
        LDS::TLDSValuePtr<FName> Body;
        LDS::TLDSObjectArrayPtr<TooltipItem> Items;
    };
}
