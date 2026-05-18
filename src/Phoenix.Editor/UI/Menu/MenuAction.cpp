#include "MenuAction.h"

#include "MenuContext.h"

Phoenix::UI::UIAction Phoenix::UI::MenuAction::ConvertToUIAction(const MenuContext& context) const
{
    UIAction action;

    if (OnExecuteAction)
    {
        action.OnExecuteAction = [executeAction = OnExecuteAction, context]
        {
            executeAction(context);
        };
    }

    if (OnCanExecuteAction)
    {
        action.OnCanExecuteAction = [canExecuteAction = OnCanExecuteAction, context]
        {
            return canExecuteAction(context);
        };
    }

    if (OnGetActionCheckState)
    {
        action.OnIsActionChecked = [getActionCheckState = OnGetActionCheckState, context]
        {
            return getActionCheckState(context);
        };
    }
    
    if (OnIsActionVisible)
    {
        action.OnIsActionVisible = [isActionVisible = OnIsActionVisible, context]
        {
            return isActionVisible(context);
        };
    }

    return action;
}
