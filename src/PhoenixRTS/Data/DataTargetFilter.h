
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataTagFilter.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API TargetFilter
    {
        TagFilter TagFilter;
        static bool Read(const LDS::LDSReadObjectArgs& args, TargetFilter& outItem);
    };

    struct PHOENIX_RTS_API TargetFilterPtr : LDS::TLDSObjectPtr<TargetFilter>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TargetFilter);

        TagFilterPtr Tags() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(TargetFilter)
}
