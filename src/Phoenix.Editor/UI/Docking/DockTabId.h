#pragma once

#include <cstdint>
#include <string>

namespace Phoenix::UI
{
    struct DockTabId
    {
        std::string TabType;
        uint32_t InstanceId = 0;

        bool operator==(const DockTabId& other) const;

        bool operator!=(const DockTabId& other) const;

        std::string ToString() const;
    };
}

namespace std
{
    template <>
    struct hash<Phoenix::UI::DockTabId>
    {
        std::size_t operator()(const Phoenix::UI::DockTabId& dockTabId) const noexcept
        {
            return std::hash<std::string>()(dockTabId.TabType) ^ std::hash<uint32_t>()(dockTabId.InstanceId);
        }
    };
}