#pragma once

#include "Phoenix/Reflection/Registration.h"
#include "JsonModelObject.h"
#include "History/Action.h"

namespace Phoenix::PropertyGrid
{
    class UpdateJsonModelObjectAction : public Phoenix::Action
    {
        PHX_DECLARE_TYPE_DERIVED(UpdateJsonModelObjectAction, Phoenix::Action)
    public:

        UpdateJsonModelObjectAction(
            const std::shared_ptr<JsonModelObject>& object,
            const nlohmann::json::json_pointer& pointer,
            nlohmann::json newValue);

    protected:

        bool CanUndo() const override;
        bool CanRedo() const override;

        void PerformUndo() override;
        void PerformRedo() override;

        std::shared_ptr<JsonModelObject> Target;

        nlohmann::json::json_pointer OldPointer;
        nlohmann::json OldValue;

        nlohmann::json::json_pointer NewPointer;
        nlohmann::json NewValue;
    };
}
