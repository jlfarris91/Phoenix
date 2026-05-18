#include "CommandInfo.h"

#include <memory>

std::shared_ptr<const Phoenix::UI::CommandInfo> Phoenix::UI::CommandInfo::CreateCommand(
    const std::string& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& description,
    const Attribute<UI::Icon>& icon,
    EUIActionType actionType)
{
    std::shared_ptr<CommandInfo> command = std::make_shared<CommandInfo>();
    command->Name = name;
    command->Label = label;
    command->Description = description;
    command->IconAttr = icon;
    command->ActionType = actionType;
    return command;
}

const std::string& Phoenix::UI::CommandInfo::GetName() const
{
    return Name;
}

std::string Phoenix::UI::CommandInfo::GetLabel() const
{
    return Label.GetValue();
}

std::string Phoenix::UI::CommandInfo::GetDescription() const
{
    return Description.GetValue();
}

Phoenix::UI::Icon Phoenix::UI::CommandInfo::GetIcon() const
{
    return IconAttr.GetValue();
}

Phoenix::UI::EUIActionType Phoenix::UI::CommandInfo::GetActionType() const
{
    return ActionType;
}
