#pragma once

#include <nlohmann/json.hpp>

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    struct PropertyDescriptor;
    struct TypeDescriptor;

    // Serialise a single reflected property to a JSON value.
    // Returns a JSON null for world-context properties and unsupported types.
    PHOENIX_SIM_API nlohmann::json PropertyToJson(const void* obj, const PropertyDescriptor& prop);

    // Serialise all registered properties of a type to a JSON object  { "fieldName": value }.
    PHOENIX_SIM_API nlohmann::json TypeToJson(const void* obj, const TypeDescriptor& desc);
}
