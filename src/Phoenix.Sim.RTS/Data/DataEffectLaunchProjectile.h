
#pragma once

#include "DataProjectile.h"
#include "PhoenixRTS/Data/DataEffect.h"

namespace Phoenix::RTS::Data
{
    enum class EEffectLaunchProjectileFlags : uint8
    {
        None = 0,
        Channeling = 1
    };
    
    struct PHOENIX_RTS_API EffectLaunchProjectile : Effect
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, EffectLaunchProjectile& outItem);
    };

    struct PHOENIX_RTS_API EffectLaunchProjectilePtr : EffectPtr
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(EffectLaunchProjectile);

        LDS::TLDSEnumFlagsPtr<EEffectLaunchProjectileFlags> Flags() const;
        EffectRefPtr ImpactEffect() const;
        EffectTargetPtr ImpactTarget() const;
        EffectRefPtr LaunchEffect() const;
        EffectTargetPtr LaunchTarget() const;
        EffectRefPtr PeriodicEffect() const;
        LDS::TimePtr PeriodicTime() const;
        ProjectileRefPtr Projectile() const;
        LDS::DistancePtr RangeMax() const;
        LDS::NamePtr TrackerLabel() const;
        ValidatorRefPtr Validator() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(EffectLaunchProjectile)
}
