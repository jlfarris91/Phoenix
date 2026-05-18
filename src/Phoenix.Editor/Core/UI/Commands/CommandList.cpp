#include "CommandList.h"

void Phoenix::UI::CommandList::BindAction(
    const std::shared_ptr<const CommandInfo>& command,
    const UIAction& action)
{
    Bindings[command] = action;
}

void Phoenix::UI::CommandList::BindAction(
    const std::shared_ptr<const CommandInfo>& command,
    const ExecuteActionFunc& executeFunc,
    const CanExecuteActionFunc& canExecuteFunc,
    const IsActionCheckedFunc& isCheckedFunc,
    const IsActionVisibleFunc& isVisibleFunc)
{
    UIAction action;
    action.OnExecuteAction = executeFunc;
    action.OnCanExecuteAction = canExecuteFunc;
    action.OnIsActionChecked = isCheckedFunc;
    action.OnIsActionVisible = isVisibleFunc;
    BindAction(command, action);
}

bool Phoenix::UI::CommandList::UnbindCommand(const std::shared_ptr<const CommandInfo>& command)
{
    if (!Bindings.contains(command))
    {
        return false;
    }

    Bindings.erase(command);
    return true;
}

const Phoenix::UI::UIAction* Phoenix::UI::CommandList::GetActionForCommand(
    const std::shared_ptr<const CommandInfo>& command) const
{
    auto iter = Bindings.find(command);
    return iter != Bindings.end() ? &iter->second : nullptr;
}
