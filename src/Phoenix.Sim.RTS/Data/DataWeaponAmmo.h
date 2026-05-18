
#pragma once

#include "Phoenix.Sim.LDS/LDSObjectModel.h"
#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataProjectile.h"

namespace Phoenix::RTS::Data
{
    enum class EWeaponAmmoRechargeMode : uint32
    {
        Sequential = (uint32)"Sequential"_n,
        Parallel = (uint32)"Parallel"_n,
        Never = (uint32)"Never"_n
    };
    
    struct PHOENIX_RTS_API WeaponAmmo
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, WeaponAmmo& outItem);
    };

    struct PHOENIX_RTS_API WeaponAmmoPtr : LDS::TLDSObjectPtr<WeaponAmmo>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(WeaponAmmo);

        LDS::Int32Ptr Initial() const;
        LDS::Int32Ptr Capacity() const;
        LDS::Int32Ptr RechargeCount() const;
        LDS::TimePtr RechargeTime() const;
        LDS::TLDSValuePtr<EWeaponAmmoRechargeMode> RechargeMode() const;
        ProjectileRefPtr Projectile() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(WeaponAmmo)
}
