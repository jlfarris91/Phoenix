#pragma once

namespace Phoenix
{
    class World;

    class PHOENIX_SIM_API IPropertyAccessor
    {
    public:
        virtual ~IPropertyAccessor() = default;

        virtual bool IsReadOnly() const = 0;
        virtual bool IsStatic() const = 0;
        virtual bool RequiresWorld() const = 0;

        virtual void Get(const void* obj, void* value, size_t len) const = 0;
        virtual void Set(void* obj, const void* value, size_t len) const = 0;

        virtual void Get(const World& world, const void* obj, void* value, size_t len) const = 0;
        virtual void Set(World& world, void* obj, const void* value, size_t len) const = 0;

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
    };

    template <class T, class TValue, class TSetterValue = const TValue&>
    struct PropertyAccessor : IPropertyAccessor
    {
        using TGetter = TValue(T::*)() const;
        using TSetter = void(T::*)(TSetterValue);

        PropertyAccessor(TGetter getter, TSetter setter = nullptr) : Getter(getter), Setter(setter) {}

        TValue Get(const void* obj) const
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(Getter);
            const T* typedObj = static_cast<const T*>(obj);
            return (typedObj->*Getter)();
        }

        void Set(void* obj, TSetterValue val) const
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(Setter);
            T* typedObj = static_cast<T*>(obj);
            return (typedObj->*Setter)(val);
        }

        bool IsReadOnly() const override
        {
            return Setter != nullptr;
        }

        bool IsStatic() const override
        {
            return false;
        }

        bool RequiresWorld() const override
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

        void Get(const World& world, const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Set(World& world, void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Initialize(void* memory) const override
        {
            TValue* typedValue = static_cast<TValue*>(memory);
            *typedValue = TValue{};
        }

        TGetter Getter = nullptr;
        TSetter Setter = nullptr;
    };

    template <class TValue, class TSetterValue = const TValue&>
    struct StaticPropertyAccessor : IPropertyAccessor
    {
        using TGetter = TValue(*)();
        using TSetter = void(*)(TSetterValue);

        StaticPropertyAccessor(TGetter getter, TSetter setter = nullptr) : Getter(getter), Setter(setter) {}

        TValue Get() const
        {
            PHX_ASSERT(Getter);
            return (*Getter)();
        }

        void Set(TSetterValue val) const
        {
            PHX_ASSERT(Setter);
            return (*Setter)(val);
        }

        bool IsReadOnly() const override
        {
            return Setter != nullptr;
        }

        bool IsStatic() const override
        {
            return true;
        }

        bool RequiresWorld() const override
        {
            return false;
        }

        void Get(const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            TValue* typedValue = static_cast<TValue*>(value);
            *typedValue = Get();
        }

        void Set(void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            const TValue* typedValue = static_cast<const TValue*>(value);
            Set(*typedValue);
        }

        void Get(const World& world, const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Set(World& world, void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Initialize(void* memory) const override
        {
            TValue* typedValue = static_cast<TValue*>(memory);
            *typedValue = TValue{};
        }

        TGetter Getter = nullptr;
        TSetter Setter = nullptr;
    };

    template <class TValue>
    struct StaticWorldPropertyAccessor : IPropertyAccessor
    {
        using TGetter = TValue(*)(const class World&);
        using TSetter = void(*)(World&, const TValue&);

        StaticWorldPropertyAccessor(TGetter getter, TSetter setter = nullptr) : Getter(getter), Setter(setter) {}

        TValue Get(const World& world) const
        {
            PHX_ASSERT(Getter);
            return (*Getter)(world);
        }

        void Set(World& world, const TValue& val) const
        {
            PHX_ASSERT(Setter);
            return (*Setter)(world, val);
        }

        bool IsReadOnly() const override
        {
            return Setter != nullptr;
        }

        bool IsStatic() const override
        {
            return true;
        }

        bool RequiresWorld() const override
        {
            return true;
        }

        void Get(const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Set(void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(false);
        }

        void Get(const World& world, const void* obj, void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            TValue* typedValue = static_cast<TValue*>(value);
            *typedValue = Get(world);
        }

        void Set(World& world, void* obj, const void* value, size_t len) const override
        {
            PHX_ASSERT(obj == nullptr);
            const TValue* typedValue = static_cast<const TValue*>(value);
            Set(world, *typedValue);
        }

        void Initialize(void* memory) const override
        {
            TValue* typedValue = static_cast<TValue*>(memory);
            *typedValue = TValue{};
        }

        TGetter Getter = nullptr;
        TSetter Setter = nullptr;
    };
}