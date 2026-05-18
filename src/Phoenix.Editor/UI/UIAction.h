#pragma once

namespace Phoenix::UI
{
    typedef std::function<void()>                   ExecuteActionFunc;
    typedef std::function<bool()>                   CanExecuteActionFunc;
    typedef std::function<std::optional<bool>()>    IsActionCheckedFunc;
    typedef std::function<bool()>                   IsActionVisibleFunc;

    struct UIAction
    {
        ExecuteActionFunc OnExecuteAction;
        CanExecuteActionFunc OnCanExecuteAction;
        IsActionCheckedFunc OnIsActionChecked;
        IsActionVisibleFunc OnIsActionVisible;
    };
}