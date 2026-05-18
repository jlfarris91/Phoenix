#include "ImGuiMenuRenderer.h"

#include <imgui.h>

using namespace Phoenix::UI;

ImGuiMenuRenderer::ImGuiMenuRenderer(const std::shared_ptr<IMenuManager>& menuManager)
{
    MenuManager = menuManager;
}

void ImGuiMenuRenderer::RenderMenuBar(const std::shared_ptr<Menu>& menu)
{
    if (ImGui::BeginMenuBar())
    {
        RenderMenu(menu);
        ImGui::EndMenuBar();
    }
}

void ImGuiMenuRenderer::RenderMenu(const std::shared_ptr<Menu>& menu)
{
    for (const MenuSection& section : menu->GetSections())
    {
        RenderMenuSection(section, menu);
    }
}

void ImGuiMenuRenderer::RenderMenuSection(const MenuSection& section, const std::shared_ptr<Menu>& menu)
{
    std::string label = section.GetLabel();
    if (!label.empty())
    {
        ImGui::SeparatorText(label.c_str());
    }

    for (const MenuEntry& entry : section.GetEntries())
    {
        RenderMenuEntry(entry, menu);
    }
}

void ImGuiMenuRenderer::RenderMenuEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu)
{
    EMenuEntryType entryType = entry.GetType();
    if (entryType == EMenuEntryType::MenuEntry)
    {
        if (entry.IsSubMenu())
        {
            RenderSubMenuEntry(entry, menu);
        }
        else
        {
            RenderStandardEntry(entry, menu);
        }
    }
    else if (entryType == EMenuEntryType::Separator)
    {
        ImGui::Separator();
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Unknown menu entry type: %d", static_cast<int>(entryType));
    }
}

void ImGuiMenuRenderer::RenderStandardEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu)
{
    UIAction action;
    if (auto command = entry.GetCommand())
    {
        std::shared_ptr<const CommandList> commandList;
        if (auto actionPtr = entry.GetActionForCommand(menu->GetContext(), /* out */ commandList))
        {
            action = *actionPtr;
        }
    }
    else
    {
        action = entry.GetAction().ConvertToUIAction(menu->GetContext());
    }

    if (action.OnIsActionVisible && !action.OnIsActionVisible())
    {
        return;
    }

    bool enabled = action.OnExecuteAction != nullptr;
    if (enabled && action.OnCanExecuteAction != nullptr)
    {
        enabled = action.OnCanExecuteAction();
    }

    if (action.OnIsActionChecked)
    {
        bool selected = action.OnIsActionChecked().value_or(false);
        if (ImGui::MenuItem(entry.GetLabel().c_str(), {}, &selected, enabled))
        {
            action.OnExecuteAction();
        }
    }
    else
    {
        if (ImGui::MenuItem(entry.GetLabel().c_str(), {}, false, enabled))
        {
            action.OnExecuteAction();
        }
    }
}

void ImGuiMenuRenderer::RenderSubMenuEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu)
{
    auto menuManager = MenuManager.lock();
    if (!menuManager)
    {
        return;
    }

    if (ImGui::BeginMenu(entry.GetLabel().c_str(), /* enabled */ true))
    {
        if (std::shared_ptr<Menu> subMenu = menuManager->GenerateSubMenu(menu, entry.GetName()))
        {
            RenderMenu(subMenu);
        }

        ImGui::EndMenu();
    }
}