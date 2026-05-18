
#pragma once

#include "Phoenix.Sim.RTS/DLLExport.h"
#include "Phoenix.Sim.RTS/Data/DataIcon.h"

namespace Phoenix::RTS::Data
{
    enum class PHOENIX_RTS_API EMinimapIconFlags : uint8
    {
        None = 0,
        UseEntityFacing = 1,
        UseTeamColor = 2
    };
    
    struct PHOENIX_RTS_API MinimapIcon
    {
        FName Asset;
        EMinimapIconFlags Flags = EMinimapIconFlags::None;
        Icon Icon;
        uint32 PixelSize = 0;

        static bool Read(const LDS::LDSReadObjectArgs& args, MinimapIcon& outItem);
    };

    struct PHOENIX_RTS_API MinimapIconPtr : LDS::TLDSObjectPtr<MinimapIcon>
    {
        PHX_LDS_DECLARE_OBJECT_PTR_FOR(MinimapIcon)

        LDS::NamePtr Asset() const;
        LDS::TLDSEnumFlagsPtr<EMinimapIconFlags> Flags() const;
        IconPtr Icon() const;
        LDS::UInt32Ptr PixelSize() const;
    };

    PHX_LDS_DECLARE_ADDITIONAL_OBJ_PTR_TYPES_FOR(MinimapIcon)
}
