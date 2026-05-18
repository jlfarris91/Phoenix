#pragma once

#include <string>

#include "MenuContext.h"
#include "MenuDelegates.h"
#include "MenuEntry.h"
#include "MenuInsertPosition.h"
#include "Util/Attribute.h"

namespace Phoenix::UI
{
    struct Icon;

    class MenuSection
    {
    public:

        void Initialize(
            const std::string& name,
            const Attribute<std::string>& label,
            const MenuInsertPosition& insertPosition = {});

        std::string_view GetName() const;

        const std::vector<MenuEntry>& GetEntries() const;

        std::string GetLabel() const;
        void SetLabel(const Attribute<std::string>& label);

        MenuEntry& AddEntry(const MenuEntry& entry);

        MenuEntry& AddEntry(
            const std::shared_ptr<const CommandInfo>& command,
            const Attribute<std::string>& label = {},
            const Attribute<std::string>& tooltip = {},
            const Attribute<Icon>& icon = {});

        MenuEntry& AddSeparator(const std::string& name);

        MenuEntry& AddSubMenu(
            const std::string& name,
            const Attribute<std::string>& label,
            const Attribute<std::string>& tooltip,
            const NewMenuFunc& newMenuFunc,
            const MenuAction& action,
            EUIActionType actionType,
            const Attribute<Icon>& icon = {});

        MenuEntry* FindEntry(const std::string& name);
        const MenuEntry* FindEntry(const std::string& name) const;

    private:

        friend class Menu;
        friend class MenuAssembler;

        void InitializeFrom(const MenuSection& source, const MenuContext& context);

        int32_t FindInsertIndex(const MenuEntry& entry) const;

        std::string Name;
        Attribute<std::string> Label;
        std::vector<MenuEntry> Entries;
        MenuInsertPosition InsertPosition;
        MenuContext Context;
        NewMenuFunc Construct;
    };

}
