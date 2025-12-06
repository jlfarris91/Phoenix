
#pragma once

#include "Color.h"
#include "DLLExport.h"
#include "LDSObjectModel.h"
#include "Name.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API UnitActor
    {
        FName Asset;
        Color Tint;

        static bool Read(const LDS::LDSReadObjectArgs& args, UnitActor& outItem);
    };

    struct PHOENIX_RTS_API UnitActorPtr : LDS::TLDSObjectPtr<UnitActor>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(UnitActor);
        LDS::TLDSValuePtr<FName> Asset;
        LDS::TLDSValuePtr<Color> Tint;
    };
}
