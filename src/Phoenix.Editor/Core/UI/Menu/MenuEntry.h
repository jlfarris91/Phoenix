#pragma once

#include "MenuAction.h"
#include "MenuDelegates.h"
#include "MenuEntryType.h"
#include "MenuInsertPosition.h"
#include "UI/Icon.h"
#include "UI/UIActionType.h"
#include "UI/Commands/CommandInfo.h"
#include "UI/Commands/CommandList.h"
#include "Util/Attribute.h"

namespace Phoenix::UI
{
    struct MenuEntrySubMenuData
    {
        bool bIsSubMenu = false;
        bool bOpenSubMenuOnClick = false;
        NewMenuFunc ConstructMenu;
    };

    class MenuEntry
    {
    public:

        MenuEntry() = default;
        MenuEntry(const std::string& name, EMenuEntryType type);

        static MenuEntry InitMenuEntry(
            const std::string& name,
            const Attribute<std::string>& label,
            const Attribute<std::string>& tooltip,
            const Attribute<Icon>& icon,
            const MenuAction& action,
            EUIActionType actionType = EUIActionType::Button);

        static MenuEntry InitMenuEntry(
            const std::string& name,
            const std::shared_ptr<const CommandInfo>& command,
            const Attribute<std::string>& label = {},
            const Attribute<std::string>& tooltip = {},
            const Attribute<Icon>& icon = {});

        static MenuEntry InitMenuEntry(
            const std::string& name,
            const std::shared_ptr<const CommandInfo>& command,
            const std::shared_ptr<const CommandList>& commandList,
            const Attribute<std::string>& label = {},
            const Attribute<std::string>& tooltip = {},
            const Attribute<Icon>& icon = {});

        static MenuEntry InitSubMenu(
            const std::string& name,
            const Attribute<std::string>& label,
            const Attribute<std::string>& tooltip,
            const NewMenuFunc& newMenuFunc,
            const MenuAction& action,
            EUIActionType actionType,
            const Attribute<Icon>& icon = {});
 
        static MenuEntry InitSubMenu(
            const std::string& name,
            const Attribute<std::string>& label,
            const Attribute<std::string>& tooltip,
            const NewMenuFunc& newMenuFunc,
            const Attribute<Icon>& icon = {});

        static MenuEntry InitSeparator(const std::string& name);

        const std::string& GetName() const;

        EMenuEntryType GetType() const;

        bool IsSeparator() const;

        EUIActionType GetActionType() const;

        const MenuInsertPosition& GetInsertPosition() const;

        std::string GetLabel() const;
        void SetLabel(const std::string& label);

        std::string GetTooltip() const;
        void SetTooltip(const std::string& tooltip);

        Icon GetIcon() const;
        void SetIcon(const Icon& icon);

        bool IsSubMenu() const;

        MenuEntrySubMenuData& GetSubMenuData();
        const MenuEntrySubMenuData& GetSubMenuData() const;

        const MenuAction& GetAction() const;

        std::shared_ptr<const CommandInfo> GetCommand() const;
        std::shared_ptr<const CommandList> GetCommandList() const;

        MenuEntry& SetCommandList(const std::shared_ptr<const CommandList>& commandList);

        const UIAction* GetActionForCommand(
            const MenuContext& context,
            std::shared_ptr<const CommandList>& outCommandList) const;

    private:

        friend class Menu;
        friend class MenuAssembler;

        void SetCommand(
            const std::shared_ptr<const CommandInfo>& command,
            const std::optional<std::string>& name,
            const Attribute<std::string>& label,
            const Attribute<std::string>& tooltip,
            const Attribute<Icon>& icon);

        std::string Name;
        EMenuEntryType Type = EMenuEntryType::None;
        EUIActionType ActionType = EUIActionType::None;
        MenuInsertPosition InsertPosition;
        Attribute<std::string> Label;
        Attribute<std::string> Tooltip;
        Attribute<Icon> IconAttr;
        MenuAction Action;
        std::shared_ptr<const CommandInfo> Command;
        std::shared_ptr<const CommandList> CommandListRef;
        MenuEntrySubMenuData SubMenuData;
    };
}
