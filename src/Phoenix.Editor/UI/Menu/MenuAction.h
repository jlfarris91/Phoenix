#pragma once

#include "MenuDelegates.h"
#include "UI/UIAction.h"

namespace Phoenix::UI
{
    struct MenuAction
    {
        MenuAction() = default;
        MenuAction(MenuExecuteAction&& action) : OnExecuteAction(std::move(action)) {}

        UIAction ConvertToUIAction(const MenuContext& context) const;

        MenuExecuteAction OnExecuteAction;
        MenuCanExecuteAction OnCanExecuteAction;
        MenuGetActionCheckState OnGetActionCheckState;
        MenuIsActionVisible OnIsActionVisible;
    };
}
