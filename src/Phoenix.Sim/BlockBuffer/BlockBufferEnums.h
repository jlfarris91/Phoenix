#pragma once

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API EBufferBlockType : uint8
    {
        Static,
        Dynamic,
        Scratch
    };

    enum class PHOENIX_SIM_API EBufferBlockTypeFlags : uint8
    {
        None = 0,
        Static = 1 << (uint8)EBufferBlockType::Static,
        Dynamic = 1 << (uint8)EBufferBlockType::Dynamic,
        Scratch = 1 << (uint8)EBufferBlockType::Scratch,
        All = Static | Dynamic | Scratch
    };
}