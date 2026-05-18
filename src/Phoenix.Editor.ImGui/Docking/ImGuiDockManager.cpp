#include "ImGuiDockManager.h"

#include <imgui.h>

#include "UI/Docking/DockTab.h"
#include "UI/Docking/DockTabId.h"

using namespace Phoenix::UI;

void ImGuiDockManager::Initialize(const std::shared_ptr<Phoenix::Editor>& editor)
{
    IDockManager::Initialize(editor);
}

void ImGuiDockManager::Shutdown()
{
}

void ImGuiDockManager::RegisterDockTabFactory(const std::string& tabType, const DockTabFactory& factory)
{
    TabFactories[tabType] = factory;
}

bool ImGuiDockManager::UnregisterDockTabFactory(const std::string& tabType)
{
    return TabFactories.erase(tabType) != 0;
}

std::shared_ptr<DockTab> ImGuiDockManager::FindTab(const DockTabId& tabId) const
{
    auto iter = Tabs.find(tabId);
    return iter != Tabs.end() ? iter->second : nullptr;
}

std::shared_ptr<DockTab> ImGuiDockManager::OpenTab(const std::string& tabType, bool focusDockTab)
{
    DockTabId tabId { .TabType = tabType, .InstanceId = ++InstanceIdGen };

    auto tab = CreateDockTab(tabId);
    if (!tab)
    {
        return {};
    }

    tab->OnOpened();

    if (focusDockTab)
    {
        ImGui::SetWindowFocus(tab->GetId().ToString().c_str());
    }

    return tab;
}

std::shared_ptr<DockTab> ImGuiDockManager::RestoreTab(const DockTabId& tabId, bool focusDockTab)
{
    auto tab = FindTab(tabId);

    // A DockTab instance doesn't exist yet for this id, try to create a new one.
    if (!tab)
    {
        tab = CreateDockTab(tabId);
        if (!tab)
        {
            return {};
        }
    }

    tab->OnOpened();

    if (tab && focusDockTab)
    {
        ImGui::SetWindowFocus(tab->GetId().ToString().c_str());
    }

    return tab;
}

bool ImGuiDockManager::RequestCloseTab(const DockTabId& tabId)
{
    auto tab = FindTab(tabId);
    if (!tab)
    {
        return false;
    }

    if (!CanCloseTabInternal(tab))
    {
        return false;
    }

    tab->OnClosed();

    return true;
}

bool ImGuiDockManager::CanCloseTab(const DockTabId& tabId) const
{
    auto tab = FindTab(tabId);
    if (!tab)
    {
        return false;
    }

    return CanCloseTabInternal(tab);
}

const std::unordered_map<DockTabId, std::shared_ptr<DockTab>>& ImGuiDockManager::GetTabs() const
{
    return Tabs;
}

bool ImGuiDockManager::CanCloseTabInternal(const std::shared_ptr<DockTab>& tab) const
{
    auto factoryIter = TabFactories.find(tab->GetId().TabType);
    if (factoryIter == TabFactories.end())
    {
        return true;
    }

    const DockTabFactory& factory = factoryIter->second;
    if (factory.CanClose)
    {
        return factory.CanClose(tab);
    }

    return true;
}

std::shared_ptr<DockTab> ImGuiDockManager::CreateDockTab(const DockTabId& tabId)
{
    auto factory = FindDockTabFactory(tabId.TabType);
    if (!factory)
    {
        return {};
    }

    auto tab = factory->FactoryFunc(tabId);
    if (!tab)
    {
        return {};
    }

    Tabs[tabId] = tab;

    return tab;
}

DockTabFactory* ImGuiDockManager::FindDockTabFactory(const std::string& tabType)
{
    auto factoryIter = TabFactories.find(tabType);
    return factoryIter == TabFactories.end() ? nullptr : &factoryIter->second;
}
