#pragma once

#include <cstdint>

namespace Phoenix::UI
{
    enum class EMenuEntryType : uint8_t
    {
        None,
        MenuEntry,
        Separator,
        Heading,
        EditableText,
        ComboButton,
        Widget
    };
}
