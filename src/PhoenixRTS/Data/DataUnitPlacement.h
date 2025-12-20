
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitPlacement
    {
        // FootprintPtr Footprint;
        Distance InnerRadius;
        Distance OuterRadius;
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitPlacement& outItem);
    };

    struct PHOENIX_RTS_API UnitPlacementPtr : LDS::TLDSObjectPtr<UnitPlacement>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitPlacement);
        //LDS::TLDSObjectRefPtr<Footprint> Footprint;
        LDS::TLDSValuePtr<Distance> InnerRadius() const;
        LDS::TLDSValuePtr<Distance> OuterRadius() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitPlacement)
}
