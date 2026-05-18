#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataTag.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API TagFilter
    {
        std::vector<TagPtr> Required;
        std::vector<TagPtr> Excluded;
        static bool Read(const LDS::LDSReadObjectArgs& args, TagFilter& outItem);
    };

    struct PHOENIX_RTS_API TagFilterPtr : LDS::TLDSObjectPtr<TagFilter>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TagFilter);

        TagRefArrayPtr Required() const;
        TagRefArrayPtr Excluded() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(TagFilter)
}
