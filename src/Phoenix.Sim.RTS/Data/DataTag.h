
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Tag
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Tag& outItem);
    };

    struct PHOENIX_RTS_API TagPtr : LDS::TLDSObjectPtr<Tag>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Tag);
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Tag)
}
