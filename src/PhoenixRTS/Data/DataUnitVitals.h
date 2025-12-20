
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataVital.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitVitals
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, UnitVitals& outItem);
    };

    struct PHOENIX_RTS_API UnitVitalsPtr : LDS::TLDSObjectPtr<UnitVitals>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitVitals);

        VitalPtr Health;
        VitalPtr Energy;
        VitalPtr Shield;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(UnitVitals)
}
