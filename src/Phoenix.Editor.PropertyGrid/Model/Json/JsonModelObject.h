#pragma once

#include <nlohmann/json.hpp>

#include "Phoenix/Reflection/Registration.h"
#include "Object.h"

namespace Phoenix::PropertyGrid
{
    class JsonModelObject : public Phoenix::IObject
    {
        PHX_DECLARE_TYPE_DERIVED(JsonModelObject, Phoenix::IObject)
    public:
        std::string Id;
        nlohmann::json Data;
        std::string DisplayName;
        std::string Description;
    };
}
