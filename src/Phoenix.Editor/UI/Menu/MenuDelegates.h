#pragma once

#include <functional>
#include <optional>

namespace Phoenix::UI
{
    class Menu;
    class MenuContext;

    typedef std::function<void(Menu&)>                              NewMenuFunc;
    typedef std::function<void(const MenuContext&)>                 MenuExecuteAction;
    typedef std::function<bool(const MenuContext&)>                 MenuCanExecuteAction;
    typedef std::function<std::optional<bool>(const MenuContext&)>  MenuGetActionCheckState;
    typedef std::function<bool(const MenuContext&)>                 MenuIsActionVisible;
}
