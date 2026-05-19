#include "UpdateJsonModelObjectAction.h"

namespace Phoenix::PropertyGrid
{

UpdateJsonModelObjectAction::UpdateJsonModelObjectAction(
    const std::shared_ptr<JsonModelObject>& object,
    const nlohmann::json::json_pointer& pointer,
    nlohmann::json newValue)
    : Target(object)
    , NewPointer(pointer)
    , NewValue(std::move(newValue))
{
    // Find the top-most json object that does exist in the pointer.
    // A pointer can be nested deep within a json object. Setting a value at a pointer that doesn't exist will create
    // the necessary json objects along the way, and we need to be able to undo that.
    OldPointer = pointer;
    while (!OldPointer.empty() && !object->Data.contains(OldPointer))
    {
        OldPointer = OldPointer.parent_pointer();
    }
}

bool UpdateJsonModelObjectAction::CanUndo() const
{
    return Target != nullptr && Target->Data.contains(OldPointer);
}

bool UpdateJsonModelObjectAction::CanRedo() const
{
    return Target != nullptr;
}

void UpdateJsonModelObjectAction::PerformUndo()
{
    assert(Target != nullptr);
    Target->Data[OldPointer] = OldValue;
}

void UpdateJsonModelObjectAction::PerformRedo()
{
    assert(Target != nullptr);
    Target->Data[NewPointer] = NewValue;
}

} // namespace Phoenix::PropertyGrid
