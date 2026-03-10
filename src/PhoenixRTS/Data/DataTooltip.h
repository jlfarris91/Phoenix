#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataTooltipItem.h"

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
        std::vector<TooltipItem> Items;

        static bool Read(const LDS::LDSReadObjectArgs& args, Tooltip& outItem);
    };

    struct PHOENIX_RTS_API TooltipPtr : LDS::TLDSObjectPtr<Tooltip>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Tooltip)

        LDS::TLDSValuePtr<FName> Title() const;
        LDS::TLDSValuePtr<FName> SubTitle() const;
        LDS::TLDSValuePtr<FName> Body() const;
        LDS::TLDSObjectArrayPtr<TooltipItem> Items() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(Tooltip)
}
