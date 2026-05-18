#include "MenuContext.h"

#include "UI/Commands/CommandList.h"
#include "Object.h"

using namespace Phoenix;
using namespace Phoenix::UI;

MenuContext::MenuContext(const std::shared_ptr<IObject>& object, const std::shared_ptr<UI::CommandList>& commandList)
{
    if (object)
    {
        ContextObjects.push_back(object);
    }
    if (commandList)
    {
        AppendCommandList(commandList);
    }
}

void MenuContext::AddObject(const std::shared_ptr<IObject>& object)
{
    if (std::ranges::find(ContextObjects, object) == ContextObjects.end())
    {
        ContextObjects.push_back(object);
    }
}

const std::vector<std::shared_ptr<IObject>>& MenuContext::GetContextObjects() const
{
    return ContextObjects;
}

const std::vector<std::shared_ptr<CommandList>>& MenuContext::GetCommandLists() const
{
    return CommandLists;
}

void MenuContext::AppendCommandList(const std::shared_ptr<UI::CommandList>& commandList)
{
    CommandLists.push_back(commandList);
}

const UIAction* MenuContext::GetActionForCommand(const std::shared_ptr<const CommandInfo>& command) const
{
    for (const auto& commandList : CommandLists)
    {
        if (commandList)
        {
            if (const UIAction* action = commandList->GetActionForCommand(command))
            {
                return action;
            }
        }
    }
    return nullptr;
}

const UIAction* MenuContext::GetActionForCommand(
    const std::shared_ptr<const CommandInfo>& command,
    std::shared_ptr<const UI::CommandList>& outCommandList) const
{
    for (const auto& commandList : CommandLists)
    {
        if (commandList)
        {
            if (const UIAction* action = commandList->GetActionForCommand(command))
            {
                outCommandList = commandList;
                return action;
            }
        }
    }
    return nullptr;
}
