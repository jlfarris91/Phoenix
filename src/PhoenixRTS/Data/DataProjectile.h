
#pragma once

#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixRTS/DLLExport.h"
#include "PhoenixRTS/Data/DataProjectileActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Projectile
    {
        static bool Read(const LDS::LDSReadObjectArgs& args, Projectile& outItem);
    };

    struct PHOENIX_RTS_API ProjectilePtr : LDS::TLDSObjectPtr<Projectile>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Projectile);

        ProjectileActorRefPtr Actor;
        LDS::TLDSValuePtr<Phoenix::Value> Health;
        LDS::TLDSValuePtr<Distance> Radius;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Projectile)
}
