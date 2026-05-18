#pragma once

#include <optional>

#include "DockDelegates.h"
#include "DockTabId.h"
#include "Editor/EditorService.h"

namespace Phoenix::UI
{
    class DockTab;
    struct DockTabId;

    struct DockTabFactory
    {
        DockTabFactoryFunc FactoryFunc;
        CanCloseDockTabFunc CanClose;
        std::optional<DockTabId> SpawnAfterTab;
        std::optional<std::string> SpawnInNode;
    };

    class IDockManager : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(IDockManager, IEditorService)
    public:

        virtual void RegisterDockTabFactory(const std::string& tabType, const DockTabFactory& factory) = 0;

        virtual bool UnregisterDockTabFactory(const std::string& tabType) = 0;

        virtual std::shared_ptr<DockTab> FindTab(const DockTabId& tabId) const = 0;

        virtual std::shared_ptr<DockTab> OpenTab(const std::string& tabType, bool focusDockTab) = 0;

        virtual std::shared_ptr<DockTab> RestoreTab(const DockTabId& tabId, bool focusDockTab) = 0;

        virtual bool RequestCloseTab(const DockTabId& tabId) = 0;

        virtual bool CanCloseTab(const DockTabId& tabId) const = 0;
    };
}
