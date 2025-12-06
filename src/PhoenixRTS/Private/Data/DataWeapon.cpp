
#include "Data/DataWeapon.h"

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
