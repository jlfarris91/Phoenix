
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API FogVisibility
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, FogVisibility& outItem);
    };

    struct PHOENIX_RTS_API FogVisibilityPtr : LDS::TLDSObjectPtr<FogVisibility>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(FogVisibility);
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(FogVisibility)
}
