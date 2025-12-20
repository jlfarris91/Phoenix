
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataIcon.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitArmor
    {
        Value Value;
        Icon Icon;

        static bool Read(const LDS::LDSReadObjectArgs& args, UnitArmor& outItem);
    };

    struct PHOENIX_RTS_API UnitArmorPtr : LDS::TLDSObjectPtr<UnitArmor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitArmor)

        LDS::TLDSValuePtr<Phoenix::Value> Value;
        IconPtr Icon;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitArmor)
}
