#include "WindowWidget.h"

#include <imgui.h>
#include <imgui_internal.h>

ImGuiWindowFlags WindowWidget::GetWindowFlags() const
{
    return WindowFlags;
}

void WindowWidget::SetWindowFlags(ImGuiWindowFlags flags)
{
    WindowFlags = flags;
}

ImGuiChildFlags WindowWidget::GetChildFlags() const
{
    return ChildFlags;
}

void WindowWidget::SetChildFlags(ImGuiChildFlags flags)
{
    ChildFlags = flags;
}

ImVec2 WindowWidget::GetSize() const
{
    return Size;
}

void WindowWidget::SetSize(const ImVec2& size)
{
    Size = size;
}

void WindowWidget::Open()
{
    bIsOpen = true;
}

void WindowWidget::Close()
{
    if (bCanClose)
    {
        bIsOpen = false;
    }
}

bool WindowWidget::CanClose() const
{
    return bCanClose;
}

void WindowWidget::SetCanClose(bool canClose)
{
    bCanClose = canClose;
    bIsOpen = bIsOpen || !bCanClose;
}

void WindowWidget::Render()
{
    if (ImGui::GetCurrentWindow() != nullptr)
    {
        if (ImGui::BeginChild(Id.c_str(), Size, ChildFlags, WindowFlags))
        {
            RenderChildren();
        }
        ImGui::EndChild();
    }
    else
    {
        bool* isOpenPtr = nullptr;
        if (bCanClose)
        {
            isOpenPtr = &bIsOpen;
        }

        if (ImGui::Begin(Id.c_str(), isOpenPtr, WindowFlags))
        {
            RenderChildren();
        }
        ImGui::End();
    }
}

void WindowWidget::RenderChildren()
{
    for (auto&& child : Children)
    {
        child->Render();
    }
}
