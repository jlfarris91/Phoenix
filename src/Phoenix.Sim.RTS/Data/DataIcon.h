
#pragma once

#include "Phoenix.Sim/Name.h"
#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataTooltip.h"

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

        LDS::NamePtr Asset() const;
        LDS::NamePtr DisplayName() const;
        TooltipPtr Tooltip() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(Icon)
}
