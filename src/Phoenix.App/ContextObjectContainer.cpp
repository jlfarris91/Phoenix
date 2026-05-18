#include "ContextObjectContainer.h"

#include <algorithm>

#include "Object.h"

void Phoenix::ContextObjectContainer::AddObject(const std::shared_ptr<IObject>& object)
{
    if (std::ranges::find(Objects, object) == Objects.end())
    {
        Objects.push_back(object);
    }
}

void Phoenix::ContextObjectContainer::AddObjects(const std::vector<std::shared_ptr<IObject>>& objects)
{
    for (auto&& object : objects)
    {
        AddObject(object);
    }
}

bool Phoenix::ContextObjectContainer::RemoveObject(const std::shared_ptr<IObject>& object)
{
    return !std::ranges::remove(Objects, object).empty();
}

bool Phoenix::ContextObjectContainer::HasObject(FName typeId) const
{
    return std::ranges::any_of(Objects, [&](const std::shared_ptr<IObject>& object)
    {
        return object->GetTypeDescriptor().IsA(typeId);
    });
}

std::shared_ptr<Phoenix::IObject> Phoenix::ContextObjectContainer::GetObject(FName typeId) const
{
    for (auto&& object : Objects)
    {
        if (object->GetTypeDescriptor().IsA(typeId))
        {
            return object;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Phoenix::IObject>> Phoenix::ContextObjectContainer::GetObjects(FName typeId) const
{
    std::vector<std::shared_ptr<IObject>> objects;
    objects.reserve(Objects.size());
    for (auto&& object : Objects)
    {
        if (object->GetTypeDescriptor().IsA(typeId))
        {
            objects.push_back(object);
        }
    }
    return objects;
}
