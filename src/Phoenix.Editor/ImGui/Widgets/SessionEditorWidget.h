#pragma once

#include "ImGuiWidget.h"

namespace Phoenix::UI
{
    class IDockManager;
    class ImGuiDockRenderer;

    class SessionEditorWidget : public ImGuiWidget
    {
        PHX_DECLARE_TYPE_DERIVED(SessionEditorWidget, ImGuiWidget)

    public:

        virtual void Render() override;
    };
}
