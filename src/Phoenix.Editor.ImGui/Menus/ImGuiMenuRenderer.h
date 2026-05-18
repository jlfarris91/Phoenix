#pragma once

#include <memory>

#include "UI/Menu/MenuManager.h"

namespace Phoenix::UI
{
    class ImGuiMenuRenderer
    {
    public:

        ImGuiMenuRenderer(const std::shared_ptr<IMenuManager>& menuManager);

        void RenderMenuBar(const std::shared_ptr<Menu>& menu);
        void RenderMenu(const std::shared_ptr<Menu>& menu);
        void RenderMenuSection(const MenuSection& section, const std::shared_ptr<Menu>& menu);
        void RenderMenuEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu);
        void RenderStandardEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu);
        void RenderSubMenuEntry(const MenuEntry& entry, const std::shared_ptr<Menu>& menu);

    private:
        std::weak_ptr<IMenuManager> MenuManager;
    };
}

