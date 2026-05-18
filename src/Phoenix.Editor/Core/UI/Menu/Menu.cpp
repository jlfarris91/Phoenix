#include "Menu.h"

#include "MenuManager.h"
#include "MenuUtility.h"
#include "Editor/Editor.h"

using namespace Phoenix::UI;

void Menu::Initialize(
    const std::shared_ptr<IMenuManager>& menuManager,
    const std::string& name,
    const std::string& parentName,
    const MenuContext* context)
{
    MenuManager = menuManager;
    Name = name;
    ParentName = parentName;
    if (context)
    {
        Context = *context;
    }
}

const std::string& Menu::GetMenuName() const
{
    return Name;
}

const std::string& Menu::GetParentMenuName() const
{
    return ParentName;
}

const MenuContext& Menu::GetContext() const
{
    return Context;
}

void Menu::SetContext(const MenuContext& context)
{
    Context = context;
}

MenuSection* Menu::FindSection(const std::string& name)
{
    for (auto& section : Sections)
    {
        if (section.GetName() == name)
        {
            return &section;
        }
    }
    return nullptr;
}

const MenuSection* Menu::FindSection(const std::string& name) const
{
    for (const auto& section : Sections)
    {
        if (section.GetName() == name)
        {
            return &section;
        }
    }
    return nullptr;
}

const std::vector<MenuSection>& Menu::GetSections() const
{
    return Sections;
}

MenuSection& Menu::AddSection(
    const std::string& name,
    const Attribute<std::string>& label,
    const MenuInsertPosition& insertPosition)
{
    if (MenuSection* existingSection = FindSection(name))
    {
        existingSection->SetLabel(label);
        return *existingSection;
    }

    MenuSection& newSection = Sections.emplace_back();
    newSection.Initialize(name, label, insertPosition);
    return newSection;
}

MenuSection& Menu::GetOrAddSection(const std::string& name)
{
    for (auto& section : Sections)
    {
        if (section.GetName() == name)
        {
            return section;
        }
    }
    return AddSection(name);
}

MenuSection& Menu::AddDynamicSection(
    const std::string& name,
    const NewMenuFunc& newMenuFunc,
    const MenuInsertPosition& insertPosition)
{
    MenuSection& section = AddSection(name, {}, insertPosition);
    section.Construct = newMenuFunc;
    return section;
}

std::shared_ptr<Menu> Menu::AddSubMenu(
    const std::string& sectionName,
    const std::string& subMenuName,
    const std::string& label,
    const std::string& tooltip,
    const MenuInsertPosition& insertPosition)
{
    auto menuManager = MenuManager.lock();
    if (!menuManager)
    {
        return nullptr;
    }

    MenuEntry subMenu = MenuEntry::InitSubMenu(subMenuName, label, tooltip, {});
    subMenu.InsertPosition = insertPosition;
    GetOrAddSection(sectionName).AddEntry(subMenu);
    std::string fullName = JoinMenuPaths(Name, subMenuName);
    return menuManager->FindMenu(fullName);
}

MenuEntry* Menu::FindEntry(const std::string& name)
{
    for (auto& section : Sections)
    {
        if (MenuEntry* entry = section.FindEntry(name))
        {
            return entry;
        }
    }
    return nullptr;
}

const MenuEntry* Menu::FindEntry(const std::string& name) const
{
    for (const auto& section : Sections)
    {
        if (const MenuEntry* entry = section.FindEntry(name))
        {
            return entry;
        }
    }
    return nullptr;
}

int32_t Menu::FindInsertIndex(const MenuSection& section) const
{
    MenuInsertPosition insertPosition = section.InsertPosition;
    if (insertPosition.IsDefault())
    {
        return static_cast<int32_t>(Sections.size());
    }

    if (insertPosition.Position == EMenuInsertPosition::First)
    {
        for (size_t i = 0; i < Sections.size(); ++i)
        {
            if (Sections[i].InsertPosition.Position != insertPosition.Position)
            {
                return static_cast<int32_t>(i);
            }
        }
        return static_cast<int32_t>(Sections.size());
    }

    int32_t destIndex = -1;
    for (const auto& existingSection : Sections)
    {
        if (existingSection.Name == insertPosition.Name)
        {
            destIndex = static_cast<int32_t>(&existingSection - Sections.data());
        }
    }

    if (destIndex == -1)
    {
        return -1;
    }

    if (insertPosition.Position == EMenuInsertPosition::After)
    {
        ++destIndex;
        for (size_t i = destIndex; i < Sections.size(); ++i)
        {
            if (Sections[i].InsertPosition == insertPosition)
            {
                destIndex = static_cast<int32_t>(i + 1);
            }
        }
    }

    for (size_t i = destIndex; i < Sections.size(); ++i)
    {
        if (Sections[i].InsertPosition != insertPosition)
        {
            return static_cast<int32_t>(i);
        }
    }

    return static_cast<int32_t>(Sections.size());
}

void Menu::InitializeFrom(const Menu& source, const std::string& name, const MenuContext* context)
{
    Name = name;
    ParentName = source.ParentName;
    SubMenuParent = source.SubMenuParent;
    if (context)
    {
        Context = *context;
    }
}
