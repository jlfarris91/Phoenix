#pragma once

#include <memory>
#include <string>
#include <vector>

#include "MenuContext.h"
#include "MenuDelegates.h"
#include "MenuInsertPosition.h"
#include "MenuSection.h"

namespace Phoenix::UI
{
    class IMenuManager;

    class Menu
    {
    public:

        void Initialize(
            const std::shared_ptr<IMenuManager>& menuManager,
            const std::string& name,
            const std::string& parentName = {},
            const MenuContext* context = {});

        const std::string& GetMenuName() const;
        const std::string& GetParentMenuName() const;

        const MenuContext& GetContext() const;
        void SetContext(const MenuContext& context);

        MenuSection* FindSection(const std::string& name);
        const MenuSection* FindSection(const std::string& name) const;

        const std::vector<MenuSection>& GetSections() const;

        MenuSection& AddSection(
            const std::string& name,
            const Attribute<std::string>& label = {},
            const MenuInsertPosition& insertPosition = {});

        MenuSection& GetOrAddSection(const std::string& name);

        MenuSection& AddDynamicSection(
            const std::string& name,
            const NewMenuFunc& newMenuFunc,
            const MenuInsertPosition& insertPosition = {});

        std::shared_ptr<Menu> AddSubMenu(
            const std::string& sectionName,
            const std::string& subMenuName,
            const std::string& label,
            const std::string& tooltip = {},
            const MenuInsertPosition& insertPosition = {});

        MenuEntry* FindEntry(const std::string& name);
        const MenuEntry* FindEntry(const std::string& name) const;

        template <class T>
        T* FindContextObject() const
        {
            return Context.FindObject<T>();
        }

    private:

        friend class MenuManager;
        friend class MenuAssembler;

        void InitializeFrom(const Menu& source, const std::string& name, const MenuContext* context);

        int32_t FindInsertIndex(const MenuSection& section) const;

        std::weak_ptr<IMenuManager> MenuManager;

        std::string Name;
        std::string ParentName;

        std::vector<MenuSection> Sections;

        MenuContext Context;

        std::weak_ptr<const Menu> SubMenuParent;

        std::string SubMenuSourceEntryName;
    };
}
