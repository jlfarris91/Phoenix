
#pragma once

#include "Actions.h"
#include "Platform.h"

namespace Phoenix::RTS
{
    enum class ECommandType : uint8
    {
        Order,
        Queue,
        Preempt,
        Acquire
    };

    enum class ESmartCommandType : uint8
    {
        Command,
        Queued,
        Rally
    };

    using Command = Action;
}
