
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Vital
    {
        Value Starting;
        Value Max;
        Value Regen;

        static bool Read(const LDS::LDSReadObjectArgs& args, Vital& outItem);
    };

    struct PHOENIX_RTS_API VitalPtr : LDS::TLDSObjectPtr<Vital>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Vital);

        LDS::TLDSValuePtr<Phoenix::Value> Starting;
        LDS::TLDSValuePtr<Phoenix::Value> Max;
        LDS::TLDSValuePtr<Phoenix::Value> Regen;
    };
}
