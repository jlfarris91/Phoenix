#pragma once
#include "Action.h"

namespace Phoenix
{
    class CompositeAction : public Action
    {
        PHX_DECLARE_TYPE_DERIVED(CompositeAction, Action)
    public:

        void AddAction(const std::shared_ptr<Action>& action);
        void AddActions(const std::vector<std::shared_ptr<Action>>& actions);

        const std::vector<std::shared_ptr<Action>>& GetActions() const;

        bool IsEmpty() const;

    protected:

        void PerformUndo() override;
        void PerformRedo() override;

        std::vector<std::shared_ptr<Action>> Actions;
    };
}
