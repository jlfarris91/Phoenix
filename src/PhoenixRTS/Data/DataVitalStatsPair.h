
#pragma once

#include "DataVital.h"
#include "DataVitalStats.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API VitalStatsPair
    {
        VitalPtr Vital;
        VitalStats Stats;

        static bool Read(const LDS::LDSReadObjectArgs& args, VitalStatsPair& outItem);
    };

    struct PHOENIX_RTS_API VitalStatsPairPtr : LDS::TLDSObjectPtr<VitalStatsPair>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(VitalStatsPair);

        VitalRefPtr Vital() const;
        VitalStatsPtr Stats() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(VitalStatsPair)
}
