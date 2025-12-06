
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Component
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Component& outItem);
    };

    struct PHOENIX_RTS_API ComponentPtr : LDS::TLDSObjectPtr<Component>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Component);
    };
}
