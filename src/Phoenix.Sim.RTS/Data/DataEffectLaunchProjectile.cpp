
#include "PhoenixRTS/Data/DataEffectLaunchProjectile.h"

using namespace Phoenix::RTS::Data;

bool EffectLaunchProjectile::Read(const LDS::LDSReadObjectArgs& args, EffectLaunchProjectile& outItem)
{
    bool success = Effect::Read(args, outItem);
    return success;
}

EffectLaunchProjectilePtr::EffectLaunchProjectilePtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : EffectPtr(path, flags)
{
}

Phoenix::LDS::TLDSEnumFlagsPtr<EEffectLaunchProjectileFlags> EffectLaunchProjectilePtr::Flags() const
{
    return EnumFlags<EEffectLaunchProjectileFlags>("flags");
}

EffectRefPtr EffectLaunchProjectilePtr::ImpactEffect() const
{
    return ObjectRef<EffectRefPtr>("impact_effect");
}

EffectTargetPtr EffectLaunchProjectilePtr::ImpactTarget() const
{
    return Object<EffectTargetPtr>("impact_target");
}

EffectRefPtr EffectLaunchProjectilePtr::LaunchEffect() const
{
    return ObjectRef<EffectRefPtr>("launch_effect");
}

EffectTargetPtr EffectLaunchProjectilePtr::LaunchTarget() const
{
    return Object<EffectTargetPtr>("launch_target");
}

EffectRefPtr EffectLaunchProjectilePtr::PeriodicEffect() const
{
    return ObjectRef<EffectRefPtr>("periodic_effect");
}

Phoenix::LDS::TimePtr EffectLaunchProjectilePtr::PeriodicTime() const
{
    return Value<Time>("periodic_time");
}

ProjectileRefPtr EffectLaunchProjectilePtr::Projectile() const
{
    return ObjectRef<ProjectileRefPtr>("projectile");
}

Phoenix::LDS::DistancePtr EffectLaunchProjectilePtr::RangeMax() const
{
    return Value<Distance>("range_max");
}

Phoenix::LDS::NamePtr EffectLaunchProjectilePtr::TrackerLabel() const
{
    return Value<FName>("tracker_label");
}

ValidatorRefPtr EffectLaunchProjectilePtr::Validator() const
{
    return ObjectRef<ValidatorRefPtr>("validator");
}
