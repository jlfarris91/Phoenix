
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataTag.h"

namespace Phoenix::RTS::Data
{
    enum class ETagBonusFlags : uint8
    {
        None = 0,
        AmountIsScalar = 1
    };

    struct PHOENIX_RTS_API TagBonus
    {
        Value Amount;
        ETagBonusFlags Flags = ETagBonusFlags::None;
        TagPtr Tag;

        static bool Read(const LDS::LDSReadObjectArgs& args, TagBonus& outItem);
    };

    struct PHOENIX_RTS_API TagBonusPtr : LDS::TLDSObjectPtr<TagBonus>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(TagBonus);

        LDS::ValuePtr Amount() const;
        LDS::TLDSEnumFlagsPtr<ETagBonusFlags> Flags() const;
        TagRefPtr Tag() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(TagBonus)
}
