
#pragma once

#include "DataTooltipItem.h"
#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"

namespace Phoenix::LDS
{
    struct LDSReadObjectArgs;
}

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Tooltip
    {
        FName Title;
        FName SubTitle;
        FName Body;
        TArray2<TooltipItem> Items;

        static bool Read(const LDS::LDSReadObjectArgs& args, Tooltip& outItem);
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
