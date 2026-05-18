#pragma once

#include "UI/Docking/DockManager.h"

namespace Phoenix::UI
{
    class ImGuiDockManager : public IDockManager
    {
        PHX_DECLARE_TYPE_DERIVED(ImGuiDockManager, IDockManager)

    public:

        void Initialize(const std::shared_ptr<Phoenix::Editor>& editor) override;
        void Shutdown() override;

        virtual void RegisterDockTabFactory(const std::string& tabType, const DockTabFactory& factory) override;

        virtual bool UnregisterDockTabFactory(const std::string& tabType) override;

        virtual std::shared_ptr<DockTab> FindTab(const DockTabId& tabId) const override;

        virtual std::shared_ptr<DockTab> OpenTab(const std::string& tabType, bool focusDockTab) override;

        virtual std::shared_ptr<DockTab> RestoreTab(const DockTabId& tabId, bool focusDockTab) override;

        virtual bool RequestCloseTab(const DockTabId& tabId) override;

        virtual bool CanCloseTab(const DockTabId& tabId) const override;

        const std::unordered_map<DockTabId, std::shared_ptr<DockTab>>& GetTabs() const;

    private:

        bool CanCloseTabInternal(const std::shared_ptr<DockTab>& tab) const;

        std::shared_ptr<DockTab> CreateDockTab(const DockTabId& tabId);

        DockTabFactory* FindDockTabFactory(const std::string& tabType);

        std::unordered_map<std::string, DockTabFactory> TabFactories;
        std::unordered_map<DockTabId, std::shared_ptr<DockTab>> Tabs;
        uint32_t InstanceIdGen = 0;
    };
}
