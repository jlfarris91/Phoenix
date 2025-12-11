
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"
#include "DataTooltip.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API MoveAbility
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, MoveAbility& outItem);
    };

    struct PHOENIX_RTS_API MoveAbilityPtr : LDS::TLDSObjectPtr<MoveAbility>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(MoveAbility)
    };
}
