#pragma once

#include <nlohmann/json.hpp>

#include "Phoenix/Reflection/Registration.h"
#include "Model/ModelDescriptor.h"

namespace Phoenix::PropertyGrid
{
    class JsonModelDescriptor : public ModelDescriptor
    {
        PHX_DECLARE_TYPE_DERIVED(JsonModelDescriptor, ModelDescriptor)
    public:

        // Gets the json blob for a target.
        const nlohmann::json* GetTargetJson(
            const std::shared_ptr<Phoenix::IObject>& target,
            const nlohmann::json::json_pointer& pointer = "/"_json_pointer) const;

        // Attempts to get the json blob at a pointer for all targets.
        // Returns:
        //  - unset if any of the targets don't have the same value for the json blob at the pointer.
        //  - nullptr if all the targets don't have a value at the pointer.
        //  - a valid pointer if all the targets have the same value for the json blob at the pointer.
        std::optional<const nlohmann::json*> GetTargetJson(const nlohmann::json::json_pointer& pointer = "/"_json_pointer) const;
    };
}
