
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitPlacement
    {
        // Footprint Footprint;
        Distance InnerRadius;
        Distance OuterRadius;
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitPlacement& outItem);
    };

    struct PHOENIX_RTS_API UnitPlacementPtr : LDS::TLDSObjectPtr<UnitPlacement>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitPlacement);
        //LDS::TLDSObjectPtr<Footprint>
        LDS::TLDSValuePtr<Distance> InnerRadius;
        LDS::TLDSValuePtr<Distance> OuterRadius;
    };
}
