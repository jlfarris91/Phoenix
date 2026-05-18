#pragma once

#include <cstdint>

namespace Phoenix::UI
{
    enum class EUIActionType : uint8_t
    {
        None,
        Button,
        ToggleButton,
        RadioButton,
        Check
    };
}
