#pragma once

#include <cstdint>
#include <string>

namespace Phoenix::UI
{
    enum class EMenuInsertPosition : uint8_t
    {
        Default,
        Before,
        After,
        First
    };

    struct MenuInsertPosition
    {
        MenuInsertPosition() = default;
        MenuInsertPosition(const std::string& name, EMenuInsertPosition position);

        bool IsDefault() const;
        bool IsBeforeOrAfter() const;

        bool operator==(const MenuInsertPosition&) const;
        bool operator!=(const MenuInsertPosition&) const;

        std::string Name;
        EMenuInsertPosition Position = EMenuInsertPosition::Default;
    };
}
