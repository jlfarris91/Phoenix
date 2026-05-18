#pragma once

#include <memory>
#include <vector>
#include "Phoenix/Reflection/TypeDescriptor.h"

namespace Phoenix
{
    class IObject;
}

namespace Phoenix::UI
{
    struct UIAction;
    class CommandInfo;
    class CommandList;
    struct MenuAction;

    class MenuContext
    {
    public:

        MenuContext() = default;
        MenuContext(const std::shared_ptr<IObject>& object, const std::shared_ptr<CommandList>& commandList = {});

        void AddObject(const std::shared_ptr<IObject>& object);

        const std::vector<std::shared_ptr<IObject>>& GetContextObjects() const;

        template <typename T>
        std::shared_ptr<T> FindObject() const
        {
            for (const auto& object : ContextObjects)
            {
                if (auto castedObject = Phoenix::Cast<T>(object))
                {
                    return castedObject;
                }
            }
            return nullptr;
        }

        const std::vector<std::shared_ptr<CommandList>>& GetCommandLists() const;

        void AppendCommandList(const std::shared_ptr<CommandList>& commandList);

        const UIAction* GetActionForCommand(const std::shared_ptr<const CommandInfo>& command) const;

        const UIAction* GetActionForCommand(
                const std::shared_ptr<const CommandInfo>& command,
                std::shared_ptr<const CommandList>& outCommandList) const;

    private:

        std::vector<std::shared_ptr<IObject>> ContextObjects;
        std::vector<std::shared_ptr<CommandList>> CommandLists;
    };
}
