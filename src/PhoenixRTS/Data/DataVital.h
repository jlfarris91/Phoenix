
#pragma once

#include "PhoenixRTS/Data/DataVitalComponent.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Vital
    {
        VitalComponentPtr Component;

        static bool Read(const LDS::LDSReadObjectArgs& args, Vital& outItem);
    };

    struct PHOENIX_RTS_API VitalPtr : LDS::TLDSObjectPtr<Vital>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Vital);

        VitalComponentRefPtr Component() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Vital)
}
