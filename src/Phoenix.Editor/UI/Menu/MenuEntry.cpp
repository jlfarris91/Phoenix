#include "MenuEntry.h"

#include "MenuAction.h"
#include "MenuContext.h"

using namespace Phoenix::UI;

MenuEntry::MenuEntry(const std::string& name, EMenuEntryType type)
    : Name(name)
    , Type(type)
{
}

MenuEntry MenuEntry::InitMenuEntry(
    const std::string& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const Attribute<UI::Icon>& icon,
    const MenuAction& action,
    EUIActionType actionType)
{
    MenuEntry entry(name, EMenuEntryType::MenuEntry);
    entry.Label = label;
    entry.Tooltip = tooltip;
    entry.IconAttr = icon;
    entry.Action = action;
    entry.ActionType = actionType;
    return entry;
}

MenuEntry MenuEntry::InitMenuEntry(
    const std::string& name,
    const std::shared_ptr<const CommandInfo>& command,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const Attribute<UI::Icon>& icon)
{
    MenuEntry entry({}, EMenuEntryType::MenuEntry);
    entry.SetCommand(command, name, label, tooltip, icon);
    entry.CommandListRef.reset();
    return entry;
}

MenuEntry MenuEntry::InitMenuEntry(
    const std::string& name,
    const std::shared_ptr<const CommandInfo>& command,
    const std::shared_ptr<const UI::CommandList>& commandList,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const Attribute<UI::Icon>& icon)
{
    MenuEntry entry({}, EMenuEntryType::MenuEntry);
    entry.SetCommand(command, name, label, tooltip, icon);
    entry.CommandListRef = commandList;
    return entry;
}

MenuEntry MenuEntry::InitSubMenu(
    const std::string& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const NewMenuFunc& newMenuFunc,
    const MenuAction& action,
    EUIActionType actionType,
    const Attribute<UI::Icon>& icon)
{
    MenuEntry entry(name, EMenuEntryType::MenuEntry);
    entry.Label = label;
    entry.Tooltip = tooltip;
    entry.IconAttr = icon;
    entry.Action = action;
    entry.ActionType = actionType;
    entry.SubMenuData.bIsSubMenu = true;
    entry.SubMenuData.ConstructMenu = newMenuFunc;
    return entry;
}

MenuEntry MenuEntry::InitSubMenu(
    const std::string& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const NewMenuFunc& newMenuFunc,
    const Attribute<UI::Icon>& icon)
{
    MenuEntry entry(name, EMenuEntryType::MenuEntry);
    entry.Label = label;
    entry.Tooltip = tooltip;
    entry.IconAttr = icon;
    entry.SubMenuData.bIsSubMenu = true;
    entry.SubMenuData.ConstructMenu = newMenuFunc;
    return entry;
}

MenuEntry MenuEntry::InitSeparator(const std::string& name)
{
    return MenuEntry(name, EMenuEntryType::Separator);
}

const std::string& MenuEntry::GetName() const
{
    return Name;
}

EMenuEntryType MenuEntry::GetType() const
{
    return Type;
}

bool MenuEntry::IsSeparator() const
{
    return Type == EMenuEntryType::Separator;
}

EUIActionType MenuEntry::GetActionType() const
{
    return ActionType;
}

const MenuInsertPosition& MenuEntry::GetInsertPosition() const
{
    return InsertPosition;
}

std::string MenuEntry::GetLabel() const
{
    return Label.GetValue();
}

void MenuEntry::SetLabel(const std::string& label)
{
    Label = label;
}

std::string MenuEntry::GetTooltip() const
{
    return Tooltip.GetValue();
}

void MenuEntry::SetTooltip(const std::string& tooltip)
{
    Tooltip = tooltip;
}

Icon MenuEntry::GetIcon() const
{
    return IconAttr.GetValue();
}

void MenuEntry::SetIcon(const UI::Icon& icon)
{
    IconAttr = icon;
}

bool MenuEntry::IsSubMenu() const
{
    return SubMenuData.bIsSubMenu;
}

MenuEntrySubMenuData& MenuEntry::GetSubMenuData()
{
    return SubMenuData;
}

const MenuEntrySubMenuData& MenuEntry::GetSubMenuData() const
{
    return SubMenuData;
}

const MenuAction& MenuEntry::GetAction() const
{
    return Action;
}

std::shared_ptr<const CommandInfo> MenuEntry::GetCommand() const
{
    return Command;
}

std::shared_ptr<const CommandList> MenuEntry::GetCommandList() const
{
    return CommandListRef;
}

MenuEntry& MenuEntry::SetCommandList(const std::shared_ptr<const UI::CommandList>& commandList)
{
    CommandListRef = commandList;
    return *this;
}

const UIAction* MenuEntry::GetActionForCommand(
    const MenuContext& context,
    std::shared_ptr<const UI::CommandList>& outCommandList) const
{
    if (!Command)
    {
        return nullptr;
    }

    if (!CommandListRef)
    {
        return context.GetActionForCommand(Command, outCommandList);
    }

    const UIAction* action = CommandListRef->GetActionForCommand(Command);
    if (!action)
    {
        return nullptr;
    }

    outCommandList = CommandListRef;
    return action;
}

void MenuEntry::SetCommand(
    const std::shared_ptr<const CommandInfo>& command,
    const std::optional<std::string>& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const Attribute<UI::Icon>& icon)
{
    Command = command;
    Name = name.has_value() ? name.value() : command->GetName();
    Label = label.HasValue() ? label.GetValue() : command->GetLabel();
    Tooltip = tooltip.HasValue() ? tooltip.GetValue() : command->GetDescription();
    IconAttr = icon.HasValue() ? icon.GetValue() : command->GetIcon();
}
