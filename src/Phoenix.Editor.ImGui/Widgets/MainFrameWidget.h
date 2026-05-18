#pragma once

#include "ImGuiWidget.h"

namespace Phoenix
{
    class Editor;
}

namespace Phoenix::UI
{
    class ImGuiMenuRenderer;
    class ImGuiDockRenderer;

    class MainFrameWidget : public ImGuiWidget
    {
        PHX_DECLARE_TYPE_DERIVED(MainFrameWidget, ImGuiWidget)

    public:

        MainFrameWidget(const ImGuiWidgetContext& context);

        void Render() override;
        void RenderMainMenuBar();
        void RenderDocking();

    private:

        std::weak_ptr<Editor> Editor;
        std::shared_ptr<ImGuiMenuRenderer> MenuRenderer;
        std::shared_ptr<ImGuiDockRenderer> DockRenderer;
    };
}
