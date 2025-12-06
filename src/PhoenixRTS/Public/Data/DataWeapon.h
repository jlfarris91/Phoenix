
#pragma once

#include "DLLExport.h"
#include "LDSObjectModel.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Weapon
    {
        static bool Read(const LDS::LDSReadObjectArgs& context, Weapon& outItem);
    };

    struct PHOENIX_RTS_API WeaponPtr : LDS::TLDSObjectPtr<Weapon>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Weapon);
    };
}
