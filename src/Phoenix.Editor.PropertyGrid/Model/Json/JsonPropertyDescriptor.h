#pragma once

#include <nlohmann/json.hpp>

#include "Phoenix/Reflection/Registration.h"
#include "Object.h"
#include "Model/PropertyDescriptor.h"

namespace Phoenix { class Action; }

namespace Phoenix::PropertyGrid
{
    class JsonPropertyDescriptor : public PropertyDescriptor
    {
        PHX_DECLARE_TYPE_DERIVED(JsonPropertyDescriptor, PropertyDescriptor)
    public:

        const nlohmann::json::json_pointer& GetPointer() const;
        void SetPointer(const nlohmann::json::json_pointer& pointer);

        // Gets the json blob for a target.
        const nlohmann::json* GetTargetJson(const std::shared_ptr<Phoenix::IObject>& target) const;

        // Attempts to get the json blob at a pointer for all targets.
        // Returns:
        //  - unset: if any of the targets don't have the same value for the json blob at the pointer.
        //  - nullptr: if all the targets don't have a value at the pointer.
        //  - a valid pointer: if all the targets have the same value for the json blob at the pointer.
        std::optional<const nlohmann::json*> GetTargetJson() const;

    protected:

        std::shared_ptr<Phoenix::Action> CreateSetValueAction(
            const nlohmann::json::json_pointer& pointer,
            const nlohmann::json& json) const;

        nlohmann::json::json_pointer Pointer;
    };
}
