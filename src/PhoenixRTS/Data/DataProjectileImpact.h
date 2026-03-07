
#pragma once

#include "PhoenixRTS/Data/DataFXActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API ProjectileImpact
    {
        FXActorPtr FX;
        Vec3 Offset;
        FName Socket;

        static bool Read(const LDS::LDSReadObjectArgs& args, ProjectileImpact& outItem);
    };

    struct PHOENIX_RTS_API ProjectileImpactPtr : LDS::TLDSObjectPtr<ProjectileImpact>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(ProjectileImpact)

        FXActorRefPtr FX() const;
        LDS::Vec3Ptr Offset() const;
        LDS::NamePtr Socket() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(ProjectileImpact)
}
