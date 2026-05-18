#include "DockTabId.h"

#include <format>

bool Phoenix::UI::DockTabId::operator==(const DockTabId& other) const
{
    return TabType == other.TabType && InstanceId == other.InstanceId;
}

bool Phoenix::UI::DockTabId::operator!=(const DockTabId& other) const
{
    return !(*this == other);
}

std::string Phoenix::UI::DockTabId::ToString() const
{
    return std::format("{}_{}", TabType, InstanceId);
}
