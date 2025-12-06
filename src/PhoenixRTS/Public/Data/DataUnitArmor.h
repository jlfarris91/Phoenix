
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "DataIcon.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitArmor
    {
        Value Value;
        Icon Icon;

        static bool Read(const LDS::LDSReadObjectArgs& context, UnitArmor& outItem);
    };

    struct PHOENIX_RTS_API UnitArmorPtr : LDS::TLDSObjectPtr<UnitArmor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitArmor)

        LDS::TLDSValuePtr<Phoenix::Value> Value;
        LDS::TLDSObjectPtr<Icon> Icon;
    };
}
