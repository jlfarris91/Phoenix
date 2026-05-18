#include "CompositeAction.h"

#include <cassert>
#include <ranges>

void Phoenix::CompositeAction::AddAction(const std::shared_ptr<Action>& action)
{
    assert(action->GetState() == EActionState::Undone);
    assert(State == EActionState::Undone);
    Actions.push_back(action);
}

void Phoenix::CompositeAction::AddActions(const std::vector<std::shared_ptr<Action>>& actions)
{
    for (const auto& action : actions)
    {
        AddAction(action);
    }
}

const std::vector<std::shared_ptr<Phoenix::Action>>& Phoenix::CompositeAction::GetActions() const
{
    return Actions;
}

bool Phoenix::CompositeAction::IsEmpty() const
{
    return Actions.empty();
}

void Phoenix::CompositeAction::PerformUndo()
{
    for (const auto& action : std::ranges::reverse_view(Actions))
    {
        action->Undo();
    }
    State = EActionState::Undone;
}

void Phoenix::CompositeAction::PerformRedo()
{
    for (const auto& action : Actions)
    {
        action->Redo();
    }
    State = EActionState::Done;
}
