#pragma once

#include <functional>
#include <memory>

namespace Phoenix::UI
{
    struct DockTabId;
    class DockTab;

    typedef std::function<std::shared_ptr<DockTab>(const DockTabId&)> DockTabFactoryFunc;
    typedef std::function<void(const std::shared_ptr<DockTab>& tab)> DockTabSpawnedFunc;
    typedef std::function<bool(const std::shared_ptr<DockTab>& tab)> CanCloseDockTabFunc;
    typedef std::function<void(const std::shared_ptr<DockTab>& tab)> DockTabClosedFunc;
}
