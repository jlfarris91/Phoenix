#include "MenuManager.h"

#include <algorithm>
#include "MenuUtility.h"

using namespace Phoenix::UI;

void MenuManager::Initialize(const std::shared_ptr<Phoenix::Editor>& editor)
{
    IMenuManager::Initialize(editor);

    Assembler.Initialize(std::static_pointer_cast<MenuManager>(shared_from_this()));
}

void MenuManager::Shutdown()
{
}

std::shared_ptr<Menu> MenuManager::RegisterMenu(
    const std::string& name,
    const std::string& parentName)
{
    if (auto existingMenu = FindMenu(name))
    {
        // TODO (jfarris): Log that a menu with the same name has already been registered. 
        return existingMenu;
    }

    std::shared_ptr<Menu> menu = std::make_shared<Menu>();
    menu->Initialize(std::static_pointer_cast<MenuManager>(shared_from_this()), name, parentName);
    RegisteredMenus[name] = menu;
    return menu;
}

std::shared_ptr<Menu> MenuManager::FindMenu(const std::string& name) const
{
    auto iter = RegisteredMenus.find(name);
    return iter != RegisteredMenus.end() ? iter->second : nullptr;
}

std::shared_ptr<Menu> MenuManager::ExtendMenu(const std::string& name)
{
    if (auto existingMenu = FindMenu(name))
    {
        return existingMenu;
    }

    std::shared_ptr<Menu> menu = NewMenu("ExtendedMenu", name);
    RegisteredMenus[name] = menu;
    return menu;
}

bool MenuManager::IsMenuRegistered(const std::string& name) const
{
    return RegisteredMenus.contains(name);
}

std::shared_ptr<Menu> MenuManager::NewMenu(
    const std::string& baseName,
    const std::string& menuName,
    const MenuContext* context)
{
    std::string uniqueMenuName = JoinMenuPaths(baseName, menuName);
    std::shared_ptr<Menu> menu = std::make_shared<Menu>();
    menu->Initialize(std::static_pointer_cast<MenuManager>(shared_from_this()), uniqueMenuName, {}, context);
    return menu;
}

std::shared_ptr<Menu> MenuManager::NewSubMenu(
    const std::shared_ptr<Menu>& parent,
    const std::string& baseName,
    const std::string& menuName)
{
    std::string uniqueMenuName = JoinMenuPaths(baseName, menuName);
    std::shared_ptr<Menu> menu = std::make_shared<Menu>();
    menu->Initialize(std::static_pointer_cast<MenuManager>(shared_from_this()), uniqueMenuName, {});
    menu->Context = parent->GetContext();
    menu->Name = {};
    menu->SubMenuParent = parent;
    return menu;
}

std::shared_ptr<Menu> MenuManager::GenerateMenu(
    const std::string& name,
    const MenuContext& context)
{
    auto iter = GeneratedMenus.find(name);
    if (iter != GeneratedMenus.end())
    {
        return iter->second;
    }

    auto hierarchy = GetMenuHierarchy(name);
    auto generatedMenu = NewMenu("TempGeneratedMenu", name, &context);
    if (!hierarchy.empty())
    {
        generatedMenu->InitializeFrom(*hierarchy.front(), hierarchy.back()->Name, &context);
        Assembler.AssembleMenuHierarchy(*generatedMenu, hierarchy);
    }
    GeneratedMenus[name] = generatedMenu;
    return generatedMenu;
}

std::shared_ptr<Menu> MenuManager::GenerateSubMenu(
    const std::shared_ptr<Menu>& parent,
    const std::string& subMenuName)
{
    if (!parent || subMenuName.empty())
    {
        return nullptr;
    }

    const MenuEntry* entry = parent->FindEntry(subMenuName);
    if (!entry)
    {
        return nullptr;
    }

    std::string subMenuFullName = JoinMenuPaths(parent->GetMenuName(), subMenuName);

    auto iter = GeneratedMenus.find(subMenuFullName);
    if (iter != GeneratedMenus.end())
    {
        return iter->second;
    }

    std::vector<std::shared_ptr<Menu>> hierarchy;
    {
        std::string baseName = parent->GetMenuName();
        while (!baseName.empty())
        {
            std::string subMenuInParentName = JoinMenuPaths(baseName, subMenuName);
            if (FindMenu(subMenuInParentName))
            {
                hierarchy = GetMenuHierarchy(subMenuInParentName);
                break;
            }
            auto menuInParent = FindMenu(baseName);
            baseName = menuInParent ? menuInParent->GetParentMenuName() : std::string();
        }
    }

    if (entry->GetSubMenuData().ConstructMenu)
    {
        auto menu = NewMenu("TempGeneratedSubMenu", subMenuFullName);
        menu->Context = parent->GetContext();
        menu->SubMenuParent = parent;
        menu->SubMenuSourceEntryName = subMenuName;
        entry->GetSubMenuData().ConstructMenu(*menu);
        menu->Name = subMenuFullName;
        hierarchy.insert(hierarchy.begin(), menu);
    }

    if (hierarchy.empty())
    {
        return nullptr;
    }

    auto generatedMenu = NewMenu("GeneratedSubMenu", subMenuFullName);
    generatedMenu->InitializeFrom(*hierarchy.front(), subMenuFullName, &parent->GetContext());
    generatedMenu->SubMenuParent = parent;
    generatedMenu->SubMenuSourceEntryName = subMenuName;
    Assembler.AssembleMenuHierarchy(*generatedMenu, hierarchy);
    GeneratedMenus[generatedMenu->Name] = generatedMenu;
    return generatedMenu;
}

std::vector<std::shared_ptr<Menu>> MenuManager::GetGeneratedMenus() const
{
    std::vector<std::shared_ptr<Menu>> generatedMenus;

    for (auto && [name, menu] : GeneratedMenus)
    {
        generatedMenus.push_back(menu);
    }

    return generatedMenus;
}

std::vector<std::shared_ptr<Menu>> MenuManager::GetMenuHierarchy(const std::string& name) const
{
    std::vector<std::shared_ptr<Menu>> hierarchy;

    std::string currentMenuName = name;
    while (!currentMenuName.empty())
    {
        auto currentMenu = FindMenu(currentMenuName);
        if (!currentMenu)
        {
            // TODO (jfarris): log failed to find menu
            break;
        }

        if (std::ranges::find(hierarchy, currentMenu) != hierarchy.end())
        {
            // TODO (jfarris): log circular menu hierarchy
            break;
        }

        hierarchy.push_back(currentMenu);
        currentMenuName = currentMenu->GetParentMenuName();
    }

    std::ranges::reverse(hierarchy);
    return hierarchy;
}
