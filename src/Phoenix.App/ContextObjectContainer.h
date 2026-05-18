#pragma once

#include <memory>
#include <vector>
#include "Phoenix/Reflection/TypeName.h"
#include "Phoenix/Reflection/TypeDescriptor.h"

namespace Phoenix
{
    class IObject;

    class ContextObjectContainer
    {
    public:
        virtual ~ContextObjectContainer() = default;

        void AddObject(const std::shared_ptr<IObject>& object);
        void AddObjects(const std::vector<std::shared_ptr<IObject>>& objects);
        bool RemoveObject(const std::shared_ptr<IObject>& object);

        bool HasObject(FName typeId) const;

        template <class T>
        bool HasObject() const
        {
            return HasObject(StaticTypeName<T>::TypeId);
        }

        std::shared_ptr<IObject> GetObject(FName typeId) const;

        template <class T>
        std::shared_ptr<T> GetObject() const
        {
            return Phoenix::Cast<T>(GetObject(StaticTypeName<T>::TypeId));
        }

        std::vector<std::shared_ptr<IObject>> GetObjects(FName typeId) const;

        template <class T>
        std::vector<std::shared_ptr<T>> GetObjects() const
        {
            std::vector<std::shared_ptr<T>> objects;
            objects.reserve(Objects.size());
            for (auto&& object : Objects)
            {
                if (auto typedObject = Phoenix::Cast<T>(object))
                {
                    objects.push_back(typedObject);
                }
            }
            return objects;
        }

    private:
        std::vector<std::shared_ptr<IObject>> Objects;
    };
}
