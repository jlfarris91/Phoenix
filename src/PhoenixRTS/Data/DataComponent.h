
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Component
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Component& outItem);
    };

    struct PHOENIX_RTS_API ComponentPtr : LDS::TLDSObjectPtr<Component>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Component);

        LDS::NamePtr TypeId() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Component)
}
