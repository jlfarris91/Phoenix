
#pragma once

#include "Phoenix.Sim.RTS/Data/DataComponent.h"
#include "Phoenix.Sim.RTS/Data/DataEffect.h"
#include "Phoenix.Sim.RTS/Data/DataTargetFilter.h"
#include "Phoenix.Sim.RTS/Data/DataIcon.h"
#include "Phoenix.Sim.RTS/Data/DataWeaponAmmo.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EWeaponFlags : uint32
    {
        Melee = (uint32)"Melee"_n,
        Channeled = (uint32)"Channeled"_n
    };
    
    struct PHOENIX_RTS_API Weapon
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Weapon& outItem);
    };

    struct PHOENIX_RTS_API WeaponPtr : LDS::TLDSObjectPtr<Weapon>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Weapon);

        TargetFilterPtr AcquireFilter() const;
        WeaponAmmoRefPtr Ammo() const;
        LDS::AnglePtr FacingArcMin() const;
        LDS::AnglePtr FacingArcSlop() const;
        LDS::TimePtr BackSwingTime() const;
        LDS::TLDSEnumFlagsPtr<EWeaponFlags> Flags() const;
        IconPtr Icon() const;
        LDS::TimePtr Period() const;
        LDS::TimePtr PeriodRandDelayMin() const;
        LDS::TimePtr PeriodRandDelayMax() const;
        LDS::TimePtr PreSwingInitial() const;
        LDS::TimePtr PreSwingRepeat() const;
        LDS::TimePtr PreSwingCooldown() const;
        LDS::DistancePtr RangeAcquire() const;
        LDS::DistancePtr RangeMin() const;
        LDS::DistancePtr RangeMax() const;
        LDS::DistancePtr RangeSlop() const;
        LDS::TimePtr SwingTime() const;
        TargetFilterPtr TargetFilter() const;
        ComponentRefPtr Component() const;
        EffectRefPtr Effect() const;
        ValidatorRefPtr Validator() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Weapon)
}
