#include "ActionManager.h"

#include "Action.h"
#include "CompositeAction.h"

bool Phoenix::IActionManager::CanPerformAction(const std::shared_ptr<Action>& action) const
{
    return action->CanRedo();
}

void Phoenix::IActionManager::SetActionId(Action& action, uint32_t id)
{
    action.ActionId = id;
}

Phoenix::ActionManager::ActionManager(CanPerformActionFunc onCanPerformAction)
    : OnCanPerformAction(std::move(onCanPerformAction))
{
}

bool Phoenix::ActionManager::CanPerformAction(const std::shared_ptr<Action>& action) const
{
    return action->CanRedo() && (OnCanPerformAction == nullptr || OnCanPerformAction(action));
}

bool Phoenix::ActionManager::AddAction(const std::shared_ptr<Action>& action)
{
    if (!CanPerformAction(action))
    {
        if (action->GetState() == EActionState::Undone)
        {
            action->Redo();
        }
        return false;
    }

    SetActionId(*action, ++ActionIdGen);

    ResetStack(RedoStack);

    if (action->GetState() != EActionState::Done)
    {
        action->Redo();
    }

    UndoStack.push_back(action);

    ActionPerformedEvent.Broadcast(action);
    RedoActionEvent.Broadcast(action);

    return true;
}

bool Phoenix::ActionManager::AddActions(const std::vector<std::shared_ptr<Action>>& actions)
{
    if (actions.empty())
    {
        return false;
    }

    if (actions.size() <= 1)
    {
        return AddAction(actions.front());
    }

    auto compositeAction = std::make_shared<CompositeAction>();
    for (const auto& action : actions)
    {
        // Pick the first action with a description? Is this good?
        if (compositeAction->GetDescription().empty() && !action->GetDescription().empty())
        {
            compositeAction->SetDescription(action->GetDescription());
        }

        compositeAction->AddAction(action);
    }

    return AddAction(compositeAction);
}

bool Phoenix::ActionManager::CanUndo() const
{
    return !UndoStack.empty() && UndoStack.back()->CanUndo();
}

bool Phoenix::ActionManager::CanRedo() const
{
    return !RedoStack.empty() && UndoStack.back()->CanRedo();
}

void Phoenix::ActionManager::Undo()
{
    if (!CanUndo())
    {
        return;
    }

    auto action = UndoStack.back();
    UndoStack.pop_back();

    action->Undo();

    ActionPerformedEvent.Broadcast(action);
    UndoActionEvent.Broadcast(action);
}

void Phoenix::ActionManager::Redo()
{
    if (!CanRedo())
    {
        return;
    }

    auto action = RedoStack.back();
    RedoStack.pop_back();

    action->Redo();

    ActionPerformedEvent.Broadcast(action);
    RedoActionEvent.Broadcast(action);
}

Phoenix::ActionEvent& Phoenix::ActionManager::OnActionPerformedEvent()
{
    return ActionPerformedEvent;
}

Phoenix::ActionEvent& Phoenix::ActionManager::OnUndoAction()
{
    return UndoActionEvent;
}

Phoenix::ActionEvent& Phoenix::ActionManager::OnRedoAction()
{
    return RedoActionEvent;
}

uint32_t Phoenix::ActionManager::GetUndoStack(std::vector<std::shared_ptr<const Action>>& outActions) const
{
    uint32_t count = 0;
    for (const auto& action : UndoStack)
    {
        outActions.push_back(action);
        ++count;
    }
    return count;
}

uint32_t Phoenix::ActionManager::GetRedoStack(std::vector<std::shared_ptr<const Action>>& outActions) const
{
    uint32_t count = 0;
    for (const auto& action : RedoStack)
    {
        outActions.push_back(action);
        ++count;
    }
    return count;
}

uint32_t Phoenix::ActionManager::GetCurrentActionId() const
{
    return UndoStack.empty() ? 0 : UndoStack.back()->GetId();
}

void Phoenix::ActionManager::ResetStack(std::vector<std::shared_ptr<Action>>& stack)
{
    for (auto& action : stack)
    {
        SetActionId(*action, -1);
    }
    stack.clear();
}
