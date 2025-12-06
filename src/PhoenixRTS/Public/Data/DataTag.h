
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Tag
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, Tag& outItem);
    };

    struct PHOENIX_RTS_API TagPtr : LDS::TLDSObjectPtr<Tag>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Tag);
    };
}
