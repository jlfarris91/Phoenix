#include "ImGuiDockRenderer.h"

#include <format>
#include <ranges>

#include "ImGuiDockManager.h"
#include "ImGuiDockTab.h"
#include "Widgets/ImGuiWidget.h"
#include "Editor/Editor.h"
#include "UI/Docking/DockTab.h"
using namespace Phoenix::UI;

ImGuiDockRenderer::ImGuiDockRenderer(const std::shared_ptr<IDockManager>& dockManager)
{
    DockManager = dockManager;
}

void ImGuiDockRenderer::Render()
{
    auto dockManager = DockManager.lock();
    if (!dockManager)
    {
        return;
    }

    std::shared_ptr<ImGuiDockManager> imguiDockManager = Phoenix::Cast<ImGuiDockManager>(dockManager);
    if (!imguiDockManager)
    {
        return;
    }

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoDocking             |
        ImGuiWindowFlags_NoTitleBar            |
        ImGuiWindowFlags_NoCollapse            |
        ImGuiWindowFlags_NoResize              |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus            ;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.0f, 0.0f));
    ImGui::Begin("##RootDockSpace", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    // Render each dock space
    {
        auto dockSpaceId = ImGui::GetID("Foobar");

        // Render the dock space
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Render each tab window
        for (const auto& tab : imguiDockManager->GetTabs() | std::views::values)
        {
            if (std::shared_ptr<ImGuiDockTab> imguiTab = Phoenix::Cast<ImGuiDockTab>(tab))
            {
                RenderTabWindow(imguiTab);
            }
        }
    }

    ImGui::End();
}

void ImGuiDockRenderer::RenderTabWindow(const std::shared_ptr<ImGuiDockTab>& tab) const
{
    auto dockManager = DockManager.lock();
    if (!dockManager)
    {
        return;
    }

    bool isOpen = tab->IsOpened();
    if (!isOpen)
    {
        return;
    }

    const DockTabId& tabId = tab->GetId();

    bool wasOpen = isOpen;
    bool* isOpenPtr = &isOpen;

    if (!dockManager->CanCloseTab(tabId))
    {
        isOpenPtr = nullptr;
    }

    if (ImGui::Begin(tabId.ToString().c_str(), isOpenPtr))
    {
        if (auto widget = tab->GetContentWidget())
        {
            widget->Render();
        }
        else
        {
            // TODO (jfarris): render the tab contents
            ImGui::Text("Tab type: %s", tabId.TabType.c_str());
            ImGui::Text("Instance ID: %u", tabId.InstanceId);
        }
    }
    ImGui::End();

    if (!isOpen && wasOpen)
    {
        dockManager->RequestCloseTab(tabId);
    }
}
