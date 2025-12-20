
#include "PhoenixRTS/Data/DataWeaponAmmo.h"

using namespace Phoenix::RTS::Data;

bool WeaponAmmo::Read(const LDS::LDSReadObjectArgs& args, WeaponAmmo& outItem)
{
    bool success = true;
    return success;
}

WeaponAmmoPtr::WeaponAmmoPtr(const LDS::LDSRecordPath& path, LDS::ELDSRecordQueryFlags flags)
    : TLDSObjectPtr(path, flags)
{
}

Phoenix::LDS::Int32Ptr WeaponAmmoPtr::Initial() const
{
    return Value<int32>("initial");
}

Phoenix::LDS::Int32Ptr WeaponAmmoPtr::Capacity() const
{
    return Value<int32>("capacity");
}

Phoenix::LDS::Int32Ptr WeaponAmmoPtr::RechargeCount() const
{
    return Value<int32>("recharge_count");
}

Phoenix::LDS::TimePtr WeaponAmmoPtr::RechargeTime() const
{
    return Value<Time>("recharge_time");
}

Phoenix::LDS::TLDSValuePtr<EWeaponAmmoRechargeMode> WeaponAmmoPtr::RechargeMode() const
{
    return Value<EWeaponAmmoRechargeMode>("recharge_mode");
}

ProjectileRefPtr WeaponAmmoPtr::Projectile() const
{
    return ObjectRef<ProjectileRefPtr>("projectile");
}
