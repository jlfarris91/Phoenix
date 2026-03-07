
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataTagFilter.h"

namespace Phoenix::RTS::Data
{
    enum class ETargetFilterAllianceFlags : uint8
    {
        None = 0,
        Ally = 1,
        Enemy = 2,
        Neutral = 4,
        Player = 8,
        Self = 16
    };

    struct PHOENIX_RTS_API TargetFilter
    {
        TagFilter TagFilter;
        ETargetFilterAllianceFlags Alliance = ETargetFilterAllianceFlags::None;
        static bool Read(const LDS::LDSReadObjectArgs& args, TargetFilter& outItem);
    };

    struct PHOENIX_RTS_API TargetFilterPtr : LDS::TLDSObjectPtr<TargetFilter>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TargetFilter);

        TagFilterPtr Tags() const;
        LDS::TLDSEnumFlagsPtr<ETargetFilterAllianceFlags> Alliance() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(TargetFilter)
}
