#pragma once

#include "PhoenixSim/Reflection/MemberDescriptor.h"

namespace Phoenix
{
    class World;
    struct IPropertyAccessor;
    class TypeDescriptor;

    class PHOENIX_SIM_API PropertyDescriptor : public MemberDescriptor
    {
    public:

        const TypeDescriptor* GetType() const;

        void Get(const void* obj, void* value, size_t len) const;
        void Set(void* obj, const void* value, size_t len) const;

        template <class T>
        T Get(const void* obj) const
        {
            T value;
            Get(obj, &value, sizeof(T));
            return value;
        }

        template <class T>
        void Set(void* obj, const T& value) const
        {
            Set(obj, &value, sizeof(T));
        }

        void Get(const World& world, const void* obj, void* value, size_t len) const ;
        void Set(World& world, void* obj, const void* value, size_t len) const ;

        template <class T>
        T Get(const World& world, const void* obj) const
        {
            T value;
            Get(world, obj, &value, sizeof(T));
            return value;
        }

        template <class T>
        void Set(World& world, void* obj, const T& value) const
        {
            Set(world, obj, &value, sizeof(T));
        }

    private:

        template <class T>
        friend class TypeDescriptorBuilder;

        const TypeDescriptor* Type = nullptr;
        std::shared_ptr<IPropertyAccessor> Accessor;
    };
}
