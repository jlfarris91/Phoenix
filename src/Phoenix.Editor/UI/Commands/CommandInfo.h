#pragma once

#include <memory>
#include <string>

#include "UI/Icon.h"
#include "UI/UIActionType.h"
#include "Util/Attribute.h"

namespace Phoenix::UI
{
    class CommandInfo
    {
    public:

        static std::shared_ptr<const CommandInfo> CreateCommand(
            const std::string& name,
            const Attribute<std::string>& label = {},
            const Attribute<std::string>& description = {},
            const Attribute<Icon>& icon = {},
            EUIActionType actionType = EUIActionType::Button);

        const std::string& GetName() const;
        std::string GetLabel() const;
        std::string GetDescription() const;
        Icon GetIcon() const;
        EUIActionType GetActionType() const;

    private:
        std::string Name;
        Attribute<std::string> Description;
        Attribute<std::string> Label;
        Attribute<Icon> IconAttr;
        EUIActionType ActionType = EUIActionType::None;
    };
}
