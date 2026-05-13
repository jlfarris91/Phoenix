#pragma once

#include <imgui.h>

#include "PanelWidget.h"

class WindowWidget : public PanelWidget
{
public:

    ImGuiWindowFlags GetWindowFlags() const;
    void SetWindowFlags(ImGuiWindowFlags flags);

    ImGuiChildFlags GetChildFlags() const;
    void SetChildFlags(ImGuiChildFlags flags);

    ImVec2 GetSize() const;
    void SetSize(const ImVec2& size);

    void Open();
    void Close();

    bool CanClose() const;
    void SetCanClose(bool canClose);

    virtual void Render() override;

private:

    void RenderChildren();

    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None;
    ImGuiChildFlags ChildFlags = ImGuiChildFlags_None;
    ImVec2 Size;
    bool bIsOpen = false;
    bool bCanClose = true;
};
