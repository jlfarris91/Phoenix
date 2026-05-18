#pragma once

#include <memory>

#include "CommandInfo.h"
#include "UI/UIAction.h"

namespace Phoenix::UI
{
    class CommandList
    {
    public:

        void BindAction(const std::shared_ptr<const CommandInfo>& command, const UIAction& action);

        void BindAction(
            const std::shared_ptr<const CommandInfo>& command,
            const ExecuteActionFunc& executeFunc,
            const CanExecuteActionFunc& canExecuteFunc = nullptr,
            const IsActionCheckedFunc& isCheckedFunc = nullptr,
            const IsActionVisibleFunc& isVisibleFunc = nullptr);

        bool UnbindCommand(const std::shared_ptr<const CommandInfo>& command);

        const UIAction* GetActionForCommand(const std::shared_ptr<const CommandInfo>& command) const;

    private:

        std::unordered_map<std::shared_ptr<const CommandInfo>, UIAction> Bindings;
    };
}
