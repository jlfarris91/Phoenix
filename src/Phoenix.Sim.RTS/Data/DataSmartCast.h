
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"

#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataTargetFilter.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API SmartCast
    {
        int32 Priority = 0;
        TargetFilter Filter;
        static bool Read(const LDS::LDSReadObjectArgs& args, SmartCast& outItem);
    };

    struct PHOENIX_RTS_API SmartCastPtr : LDS::TLDSObjectPtr<SmartCast>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(SmartCast);

        LDS::Int32Ptr Priority() const;
        TargetFilterPtr Filter() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(SmartCast)
}
