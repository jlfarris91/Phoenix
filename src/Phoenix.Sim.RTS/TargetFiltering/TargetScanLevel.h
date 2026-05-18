#pragma once

#include "Phoenix/Platform.h"

#include "Phoenix.Sim.RTS/DLLExport.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ETargetScanLevel : uint8
    {
        None,
        Defensive,
        Offensive
    };
}
