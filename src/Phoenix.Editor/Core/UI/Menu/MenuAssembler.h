#pragma once

#include <functional>
#include <memory>

namespace Phoenix::UI
{
    class Menu;
    class MenuSection;
    class MenuContext;
    class IMenuManager;

    class MenuAssembler
    {
    public:
        void Initialize(const std::shared_ptr<IMenuManager>& menuManager);
        void AssembleMenu(Menu& menu, const Menu& source);
        void AssembleMenuHierarchy(Menu& menu, const std::vector<std::shared_ptr<Menu>>& hierarchy);
        void AssembleMenuSection(MenuSection& section, const MenuSection& source);
    private:
        std::weak_ptr<IMenuManager> MenuManager;
    };
}
