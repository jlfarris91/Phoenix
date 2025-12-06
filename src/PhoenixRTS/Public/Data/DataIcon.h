
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"
#include "DataTooltip.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Icon
    {
        FName Asset;
        FName DisplayName;
        Tooltip Tooltip;

        static bool Read(const LDS::LDSReadObjectArgs& args, Icon& outItem);
    };

    struct PHOENIX_RTS_API IconPtr : LDS::TLDSObjectPtr<Icon>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Icon)

        LDS::TLDSValuePtr<FName> Asset;
        LDS::TLDSValuePtr<FName> DisplayName;
        TooltipPtr Tooltip;
    };
}
