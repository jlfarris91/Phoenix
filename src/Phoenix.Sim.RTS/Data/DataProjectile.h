
#pragma once

#include "DataProjectileLaunch.h"
#include "DataProjectileMovement.h"
#include "Phoenix.Sim.RTS/Data/DataProjectileActor.h"
#include "Phoenix.Sim.RTS/Data/DataProjectileImpact.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API Projectile
    {
        ProjectileActorPtr Actor;
        Value Health;
        ProjectileImpact Impact;
        ProjectileLaunch Launch;
        ProjectileMovement Movement;
        
        static bool Read(const LDS::LDSReadObjectArgs& args, Projectile& outItem);
    };

    struct PHOENIX_RTS_API ProjectilePtr : LDS::TLDSObjectPtr<Projectile>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(Projectile);

        ProjectileActorRefPtr Actor() const;
        LDS::ValuePtr Health() const;
        ProjectileImpactPtr Impact() const;
        ProjectileLaunchPtr Launch() const;
        ProjectileMovementPtr Movement() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_REF_PTR_TYPES_FOR(Projectile)
}
