
#include "PhoenixRTS/Data/DataWeapon.h"

using namespace Phoenix::RTS::Data;

bool Weapon::Read(const LDS::LDSReadObjectArgs& args, Weapon& outItem)
{
    bool success = true;
    return success;
}

WeaponPtr::WeaponPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

TargetFilterPtr WeaponPtr::AcquireFilter() const
{
    return Object<TargetFilterPtr>("acquire_filter");
}

WeaponAmmoRefPtr WeaponPtr::Ammo() const
{
    return ObjectRef<WeaponAmmoRefPtr>("ammo");
}

Phoenix::LDS::AnglePtr WeaponPtr::FacingArcMin() const
{
    return Value<Angle>("facing_arc_min");
}

Phoenix::LDS::AnglePtr WeaponPtr::FacingArcSlop() const
{
    return Value<Angle>("facing_arc_slop");
}

Phoenix::LDS::TimePtr WeaponPtr::BackSwingTime() const
{
    return Value<Angle>("backswing_time");
}

Phoenix::LDS::TLDSEnumFlagsPtr<EWeaponFlags> WeaponPtr::Flags() const
{
    return EnumFlags<EWeaponFlags>("flags");
}

IconPtr WeaponPtr::Icon() const
{
    return Object<IconPtr>("icon");
}

Phoenix::LDS::TimePtr WeaponPtr::Period() const
{
    return Value<Time>("period");
}

Phoenix::LDS::TimePtr WeaponPtr::PeriodRandDelayMin() const
{
    return Value<Time>("period_rand_delay_min");
}

Phoenix::LDS::TimePtr WeaponPtr::PeriodRandDelayMax() const
{
    return Value<Time>("period_rand_delay_max");
}

Phoenix::LDS::TimePtr WeaponPtr::PreSwingInitial() const
{
    return Value<Time>("preswing_initial");
}

Phoenix::LDS::TimePtr WeaponPtr::PreSwingRepeat() const
{
    return Value<Time>("preswing_repeat");
}

Phoenix::LDS::TimePtr WeaponPtr::PreSwingCooldown() const
{
    return Value<Time>("preswing_cooldown");
}

Phoenix::LDS::DistancePtr WeaponPtr::RangeAcquire() const
{
    return Value<Distance>("range_acquire");
}

Phoenix::LDS::DistancePtr WeaponPtr::RangeMin() const
{
    return Value<Distance>("range_min");
}

Phoenix::LDS::DistancePtr WeaponPtr::RangeMax() const
{
    return Value<Distance>("range_max");
}

Phoenix::LDS::DistancePtr WeaponPtr::RangeSlop() const
{
    return Value<Distance>("range_slop");
}

Phoenix::LDS::TimePtr WeaponPtr::SwingTime() const
{
    return Value<Time>("swing_time");
}

TargetFilterPtr WeaponPtr::TargetFilter() const
{
    return Object<TargetFilterPtr>("target_filter");
}

ComponentRefPtr WeaponPtr::Component() const
{
    return ObjectRef<ComponentRefPtr>("component");
}

EffectRefPtr WeaponPtr::Effect() const
{
    return ObjectRef<EffectRefPtr>("effect");
}

ValidatorRefPtr WeaponPtr::Validator() const
{
    return ObjectRef<ValidatorRefPtr>("validator");
}
