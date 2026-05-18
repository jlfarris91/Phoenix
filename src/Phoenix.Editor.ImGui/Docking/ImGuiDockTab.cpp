#include "ImGuiDockTab.h"

Phoenix::UI::ImGuiDockTab::ImGuiDockTab(const DockTabId& id)
    : DockTab(id)
{
}

std::shared_ptr<Phoenix::UI::ImGuiWidget> Phoenix::UI::ImGuiDockTab::GetContentWidget() const
{
    return ContentWidget;
}

void Phoenix::UI::ImGuiDockTab::SetContentWidget(const std::shared_ptr<ImGuiWidget>& contentWidget)
{
    ContentWidget = contentWidget;
}
