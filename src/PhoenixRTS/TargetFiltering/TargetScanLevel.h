#pragma once

#include "PhoenixSim/Platform.h"

#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ETargetScanLevel : uint8
    {
        None,
        Defensive,
        Offensive
    };
}
