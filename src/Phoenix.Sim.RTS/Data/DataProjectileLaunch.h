
#pragma once

#include "Phoenix.Sim.RTS/Data/DataFXActor.h"

namespace Phoenix::RTS::Data
{
    struct PHOENIX_RTS_API ProjectileLaunch
    {
        FXActorPtr FX;
        Angle Pitch;
        Angle PitchRand;
        Angle Roll;
        Angle RollRand;
        Vec3 Offset;
        FName Socket;

        static bool Read(const LDS::LDSReadObjectArgs& args, ProjectileLaunch& outItem);
    };

    struct PHOENIX_RTS_API ProjectileLaunchPtr : LDS::TLDSObjectPtr<ProjectileLaunch>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(ProjectileLaunch)

        FXActorRefPtr FX() const;
        LDS::Vec3Ptr Offset() const;
        LDS::AnglePtr Pitch() const;
        LDS::AnglePtr PitchRand() const;
        LDS::AnglePtr Roll() const;
        LDS::AnglePtr RollRand() const;
        LDS::NamePtr Socket() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(ProjectileLaunch)
}
