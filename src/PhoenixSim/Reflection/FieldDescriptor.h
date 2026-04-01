#pragma once

#include "PhoenixSim/Reflection/MemberDescriptor.h"

namespace Phoenix
{
    class IFieldAccessor;
    class TypeDescriptor;

    class PHOENIX_SIM_API FieldDescriptor : public MemberDescriptor
    {
    public:

        const TypeDescriptor* GetType() const;

        uint32 GetOffset() const;

        bool GetPointer(void* object, void** outValue) const;
        bool GetPointer(const void* object, const void** outValue) const;

        template <class T>
        bool GetPointer(void* object, T** outValue) const
        {
            void* outValuePtr = nullptr;
            bool result = GetPointer(object, &outValuePtr);
            *outValue = static_cast<T*>(outValuePtr);
            return result;
        }

        template <class T>
        bool GetPointer(const void* object, T* const* outValue) const
        {
            const void* outValuePtr = nullptr;
            bool result = GetPointer(object, &outValuePtr);
            *outValue = static_cast<const T*>(outValuePtr);
            return result;
        }

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

    private:

        template <class T>
        friend class TypeDescriptorBuilder;

        const TypeDescriptor* Type = nullptr;
        std::shared_ptr<IFieldAccessor> Accessor;
    };
}
