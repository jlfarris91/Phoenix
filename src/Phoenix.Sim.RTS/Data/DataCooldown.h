
#pragma once

#include "Phoenix.Sim/LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API ECooldownScope : uint32
    {
        Ability = (uint32)"Ability"_n,
        Unit = (uint32)"Unit"_n,
        Player = (uint32)"Player"_n,
        Global = (uint32)"Global"_n,
    };

    struct PHOENIX_RTS_API Cooldown
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Cooldown& outItem);
    };

    struct PHOENIX_RTS_API CooldownPtr : LDS::TLDSObjectPtr<Cooldown>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Cooldown);

        LDS::TLDSValuePtr<ECooldownScope> Scope() const;
        LDS::TimePtr Duration() const;
        LDS::NamePtr Name() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(Cooldown)
}
