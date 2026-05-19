#include "JsonPropertyDescriptor.h"

#include "JsonModelDescriptor.h"
#include "UpdateJsonModelObjectAction.h"
#include "History/CompositeAction.h"

namespace Phoenix::PropertyGrid
{

std::shared_ptr<Phoenix::Action> JsonPropertyDescriptor::CreateSetValueAction(
    const nlohmann::json::json_pointer& pointer,
    const nlohmann::json& json) const
{
    auto modelDescriptor = GetModelDescriptor();
    assert(modelDescriptor);

    auto jsonModelDescriptor = Phoenix::Cast<JsonModelDescriptor>(modelDescriptor);
    assert(jsonModelDescriptor);

    std::vector<std::shared_ptr<JsonModelObject>> targets;
    jsonModelDescriptor->GetTargets<JsonModelObject>(targets);
    if (targets.empty())
    {
        return {};
    }

    if (targets.size() == 1)
    {
        auto action = std::make_shared<UpdateJsonModelObjectAction>(targets.front(), pointer, json);
        action->SetDescription(std::format("Set property {} on 1 object", GetDisplayName()));
        return action;
    }

    auto compositeAction = std::make_shared<Phoenix::CompositeAction>();
    compositeAction->SetDescription(std::format("Set property {} on {} objects", GetDisplayName(), targets.size()));

    for (const auto& target : targets)
    {
        auto action = std::make_shared<UpdateJsonModelObjectAction>(target, pointer, json);
        compositeAction->AddAction(action);
    }

    return compositeAction;
}

const nlohmann::json::json_pointer& JsonPropertyDescriptor::GetPointer() const
{
    return Pointer;
}

void JsonPropertyDescriptor::SetPointer(const nlohmann::json::json_pointer& pointer)
{
    Pointer = pointer;
}

const nlohmann::json* JsonPropertyDescriptor::GetTargetJson(const std::shared_ptr<Phoenix::IObject>& target) const
{
    auto modelDescriptor = GetModelDescriptor();
    assert(modelDescriptor);

    auto jsonModelDescriptor = Phoenix::Cast<JsonModelDescriptor>(modelDescriptor);
    assert(jsonModelDescriptor);

    return jsonModelDescriptor->GetTargetJson(target, Pointer);
}

std::optional<const nlohmann::json*> JsonPropertyDescriptor::GetTargetJson() const
{
    auto modelDescriptor = GetModelDescriptor();
    assert(modelDescriptor);

    auto jsonModelDescriptor = Phoenix::Cast<JsonModelDescriptor>(modelDescriptor);
    assert(jsonModelDescriptor);

    return jsonModelDescriptor->GetTargetJson(Pointer);
}

} // namespace Phoenix::PropertyGrid
