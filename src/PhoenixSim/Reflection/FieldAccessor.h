#pragma once

#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    class PHOENIX_SIM_API IFieldAccessor
    {
    public:
        virtual ~IFieldAccessor() = default;

        virtual bool IsReadOnly() const = 0;
        virtual bool IsStatic() const = 0;

        virtual uint32_t GetOffset() const = 0;

        virtual bool GetPtr(void* obj, void** value) const = 0;
        virtual bool GetPtr(const void* obj, const void** value) const = 0;

        virtual void Get(const void* obj, void* value, size_t len) const = 0;
        virtual void Set(void* obj, const void* value, size_t len) const = 0;

        virtual void Initialize(void* memory) const = 0;

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
    };

    template <class T, class TValue>
    class FieldAccessor : public IFieldAccessor
    {
    public:
        using TFieldPtr = TValue T::*;

        FieldAccessor(TFieldPtr ptr)
            : FieldPtr(ptr)
            , Offset(reinterpret_cast<std::size_t>(&(static_cast<T*>(nullptr)->*ptr)))
        {
        }

        uint32_t GetOffset() const override { return Offset; }

        virtual bool GetPtr(void* obj, void** value) const override
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(FieldPtr);
            T* typedObj = static_cast<T*>(obj);
            *value = &(typedObj->*FieldPtr);
            return true;
        }

        virtual bool GetPtr(const void* obj, const void** value) const override
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(FieldPtr);
            const T* typedObj = static_cast<const T*>(obj);
            *value = &(typedObj->*FieldPtr);
            return false;
        }

        const TValue& Get(const void* obj) const
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(FieldPtr);
            const T* typedObj = static_cast<const T*>(obj);
            return typedObj->*FieldPtr;
        }

        void Set(void* obj, const TValue& val) const
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(FieldPtr);
            T* typedObj = static_cast<T*>(obj);
            typedObj->*FieldPtr = val;
        }

        bool IsReadOnly() const override
        {
            return false;
        }

        bool IsStatic() const override
        {
            return false;
        }

        void Get(const void* obj, void* value, size_t len) const override
        {
            TValue* typedValue = static_cast<TValue*>(value);
            *typedValue = Get(obj);
        }

        void Set(void* obj, const void* value, size_t len) const override
        {
            const TValue* typedValue = static_cast<const TValue*>(value);
            Set(obj, *typedValue);
        }

        void Initialize(void* memory) const override
        {
            TValue* typedValue = static_cast<TValue*>(memory);
            *typedValue = TValue{};
        }

    private:

        TFieldPtr FieldPtr = nullptr;
        uint32_t Offset = 0;
    };

    template <class TValue>
    class StaticFieldAccessor : public IFieldAccessor
    {
        using TFieldPtr = TValue*;
        using TDecay = std::remove_cv_t<std::remove_pointer_t<TFieldPtr>>;

    public:

        StaticFieldAccessor(TFieldPtr ptr) : FieldPtr(ptr) { }

        uint32_t GetOffset() const override { return -1; }

        virtual bool GetPtr(void* obj, void** value) const override
        {
            PHX_ASSERT(obj == nullptr);
            if constexpr (!std::is_const_v<TValue>)
            {
                *value = &Get();
                return true;
            }
            return false;
        }

        virtual bool GetPtr(const void* obj, const void** value) const override
        {
            PHX_ASSERT(obj == nullptr);
            *value = &Get();
            return true;
        }

        const TDecay& Get() const
        {
            PHX_ASSERT(FieldPtr);
            return *FieldPtr;
        }

        void Set(const TDecay& val) const
        {
            PHX_ASSERT(FieldPtr);
            if constexpr (!std::is_const_v<TValue>)
            {
                *FieldPtr = val;
            }
        }

        bool IsReadOnly() const override
        {
            return false;
        }

        bool IsStatic() const override
        {
            return true;
        }

        void Get(const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            TDecay* typedValue = static_cast<TDecay*>(value);
            *typedValue = Get();
        }

        void Set(void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            const TDecay* typedValue = static_cast<const TDecay*>(value);
            Set(*typedValue);
        }

        void Initialize(void* memory) const override
        {
            TDecay* typedValue = static_cast<TDecay*>(memory);
            *typedValue = TDecay{};
        }

    private:
        TFieldPtr FieldPtr = nullptr;
    };
}