
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API VitalStats
    {
        Value Starting;
        Value Max;
        Value Regen;

        static bool Read(const LDS::LDSReadObjectArgs& args, VitalStats& outItem);
    };

    struct PHOENIX_RTS_API VitalStatsPtr : LDS::TLDSObjectPtr<VitalStats>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(VitalStats);

        LDS::ValuePtr Starting() const;
        LDS::ValuePtr Max() const;
        LDS::ValuePtr Regen() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(VitalStats)
}
