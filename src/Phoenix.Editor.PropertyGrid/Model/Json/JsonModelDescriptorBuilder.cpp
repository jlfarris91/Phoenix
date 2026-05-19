#include "JsonModelDescriptorBuilder.h"

#include "JsonModelDescriptor.h"
#include "JsonModelObject.h"
#include "JsonPrimitivePropertyDescriptor.h"
#include "JsonPropertyDescriptor.h"

namespace Phoenix::PropertyGrid
{

bool JsonModelDescriptorBuilder::CanBuild(const std::shared_ptr<Phoenix::IObject>& object) const
{
    return Phoenix::Cast<JsonModelObject>(object) != nullptr;
}

std::shared_ptr<IModelDescriptor> JsonModelDescriptorBuilder::Build(
    const std::shared_ptr<Phoenix::IObject>& object)
{
    auto jsonModelObject = Phoenix::Cast<JsonModelObject>(object);
    assert(jsonModelObject);

    auto rootModelDescriptor = std::make_shared<JsonModelDescriptor>();
    rootModelDescriptor->SetId(jsonModelObject->Id);
    rootModelDescriptor->SetDisplayName(jsonModelObject->DisplayName);
    rootModelDescriptor->SetDescription(jsonModelObject->Description);
    rootModelDescriptor->SetTargets({ object });

    ProcessJson(rootModelDescriptor, nullptr, jsonModelObject, nlohmann::json::json_pointer());

    return rootModelDescriptor;
}

void JsonModelDescriptorBuilder::ProcessJson(
    const std::shared_ptr<ModelDescriptor>& modelDescriptor,
    const std::shared_ptr<PropertyDescriptor>& parentDescriptor,
    const std::shared_ptr<JsonModelObject>& object,
    const nlohmann::json::json_pointer& pointer)
{
    const nlohmann::json* json = &object->Data[pointer];
    std::string id = pointer.to_string();
    std::string displayName = pointer.empty() ? "" : pointer.back();
    std::string description = pointer.to_string();

    if (json->is_boolean())
    {
        auto prop = std::make_shared<JsonPrimitivePropertyDescriptor<bool>>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        return;
    }

    if (json->is_number_float())
    {
        auto prop = std::make_shared<JsonPrimitivePropertyDescriptor<float>>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        return;
    }

    if (json->is_number_integer())
    {
        auto prop = std::make_shared<JsonPrimitivePropertyDescriptor<int32_t>>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        return;
    }

    if (json->is_number_unsigned())
    {
        auto prop = std::make_shared<JsonPrimitivePropertyDescriptor<uint32_t>>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        return;
    }

    if (json->is_string())
    {
        auto prop = std::make_shared<JsonPrimitivePropertyDescriptor<std::string>>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        return;
    }

    if (json->is_object())
    {
        auto prop = std::make_shared<JsonPropertyDescriptor>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        for (const auto& [key, value] : json->items())
        {
            ProcessJson(modelDescriptor, parentDescriptor, object, pointer / key);
        }
    }

    if (json->is_array())
    {
        auto prop = std::make_shared<JsonPropertyDescriptor>();
        prop->SetId(id);
        prop->SetDisplayName(displayName);
        prop->SetDescription(description);
        prop->SetModelDescriptor(modelDescriptor);
        prop->SetParentProperty(parentDescriptor);
        prop->SetPointer(pointer);
        prop->SetIsArray(true);
        for (const auto& [indexStr, value] : json->items())
        {
            uint32_t index = std::stoul(indexStr);
            ProcessJson(modelDescriptor, parentDescriptor, object, pointer / index);
        }
    }
}

} // namespace Phoenix::PropertyGrid
