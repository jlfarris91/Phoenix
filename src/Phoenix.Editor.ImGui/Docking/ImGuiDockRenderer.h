#pragma once

#include <memory>
#include <imgui.h>

#include "UI/Docking/DockManager.h"

namespace Phoenix::UI
{
    class ImGuiDockTab;

    class ImGuiDockRenderer
    {
    public:

        ImGuiDockRenderer(const std::shared_ptr<IDockManager>& dockManager);

        void Render();

    private:

        void RenderTabWindow(const std::shared_ptr<ImGuiDockTab>& tab) const;

        std::weak_ptr<IDockManager> DockManager;
        ImGuiID DockSpaceId = 0;
    };
}
