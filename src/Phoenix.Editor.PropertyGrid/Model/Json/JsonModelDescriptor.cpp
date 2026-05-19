#include "JsonModelDescriptor.h"

#include "JsonModelObject.h"

namespace Phoenix::PropertyGrid
{

const nlohmann::json* JsonModelDescriptor::GetTargetJson(
    const std::shared_ptr<Phoenix::IObject>& target,
    const nlohmann::json::json_pointer& pointer) const
{
    assert(std::ranges::find(Targets, target) != Targets.end());

    auto jsonModelObject = Phoenix::Cast<JsonModelObject>(target);
    if (!jsonModelObject)
    {
        return nullptr;
    }

    const nlohmann::json& json = jsonModelObject->Data;
    if (!json.contains(pointer))
    {
        return nullptr;
    }

    return &json.at(pointer);
}

std::optional<const nlohmann::json*> JsonModelDescriptor::GetTargetJson(
    const nlohmann::json::json_pointer& pointer) const
{
    std::optional<const nlohmann::json*> result;

    auto getValueJson = [&pointer](const std::shared_ptr<Phoenix::IObject>& target) -> const nlohmann::json*
    {
        auto jsonModelObject = Phoenix::Cast<JsonModelObject>(target);
        if (!jsonModelObject)
        {
            return nullptr;
        }

        const nlohmann::json& json = jsonModelObject->Data;
        if (!json.contains(pointer))
        {
            return nullptr;
        }

        return &json.at(pointer);
    };

    for (const auto& target : Targets)
    {
        const nlohmann::json* valueJson = getValueJson(target);

        if (!result.has_value())
        {
            result = valueJson;
            continue;
        }

        if (result.value() == nullptr && valueJson != nullptr)
        {
            result.reset();
            break;
        }

        if (*result.value() != *valueJson)
        {
            result.reset();
            break;
        }
    }

    return result;
}

} // namespace Phoenix::PropertyGrid
