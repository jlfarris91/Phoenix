#pragma once

#include "UI/Docking/DockTab.h"

namespace Phoenix::UI
{
    class ImGuiWidget;

    class ImGuiDockTab : public DockTab
    {
        PHX_DECLARE_TYPE_DERIVED(ImGuiDockTab, DockTab)

    public:

        ImGuiDockTab(const DockTabId& id);

        std::shared_ptr<ImGuiWidget> GetContentWidget() const;
        void SetContentWidget(const std::shared_ptr<ImGuiWidget>& contentWidget);

    private:

        std::shared_ptr<ImGuiWidget> ContentWidget;
    };
}
