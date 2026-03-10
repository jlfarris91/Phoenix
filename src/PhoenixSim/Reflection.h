
#pragma once

#include <memory>
#include <ranges>

#include "Flags.h"
#include "Utils.h"
#include "FixedPoint/FixedVector.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"

namespace Phoenix
{
    class World;

    enum class PHOENIX_SIM_API EPropertyValueType
    {
        Unknown,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        Bool,
        String,
        Name,
        FixedPoint,
        Vec2,
        COUNT
    };

    enum class PHOENIX_SIM_API ETypeDescriptorFlags
    {
        None = 0,
        Interface = 1,
    };

    struct PHOENIX_SIM_API DescriptorBase
    {
        virtual ~DescriptorBase() = default;

        std::string Name;
        std::unordered_map<std::string, std::string> Metadata;
    };

    struct PHOENIX_SIM_API IMethodPointer
    {
        virtual ~IMethodPointer() = default;

        virtual bool IsStatic() const = 0;
        virtual bool RequiresWorld() const = 0;

        virtual void Execute(void* obj) const = 0;
        virtual bool CanExecute(void* obj) const = 0;

        virtual void Execute(World& world, void* obj) const = 0;
        virtual bool CanExecute(const World& world, void* obj) const = 0;
    };

    struct PHOENIX_SIM_API MethodDescriptor : DescriptorBase
    {
        std::shared_ptr<IMethodPointer> MethodPointer;
    };

    template <class T>
    struct MethodPointer : IMethodPointer
    {
        using TExecutePtr = void(T::*)();
        using TCanExecutePtr = bool(T::*)() const;

        MethodPointer(TExecutePtr executePtr, TCanExecutePtr canExecutePtr = nullptr)
            : ExecutePtr(executePtr)
            , CanExecutePtr(canExecutePtr)
        {
        }

        bool IsStatic() const override
        {
            return false;
        }

        bool RequiresWorld() const override
        {
            return false;
        }

        void Execute(void* obj) const override
        {
            PHX_ASSERT(CanExecute(obj));
            T* typedObj = static_cast<T*>(obj);
            (typedObj->*ExecutePtr)();
        }

        bool CanExecute(void* obj) const override
        {
            if (!ExecutePtr) return false;
            if (!CanExecutePtr) return true;
            const T* typedObj = static_cast<const T*>(obj);
            return (typedObj->*CanExecutePtr)();
        }

        void Execute(World& world, void* obj) const override
        {
            PHX_ASSERT(false);
        }

        bool CanExecute(const World& world, void* obj) const override
        {
            PHX_ASSERT(false);
            return false;
        }

        TExecutePtr ExecutePtr = nullptr;
        TCanExecutePtr CanExecutePtr = nullptr;
    };

    template <class T>
    struct ConstMethodPointer : IMethodPointer
    {
        using TExecutePtr = void(T::*)() const;
        using TCanExecutePtr = bool(T::*)() const;

        ConstMethodPointer(TExecutePtr executePtr, TCanExecutePtr canExecutePtr = nullptr)
            : ExecutePtr(executePtr)
            , CanExecutePtr(canExecutePtr)
        {
        }

        bool IsStatic() const override
        {
            return false;
        }

        bool RequiresWorld() const override
        {
            return false;
        }

        void Execute(void* obj) const override
        {
            PHX_ASSERT(CanExecute(obj));
            const T* typedObj = static_cast<const T*>(obj);
            (typedObj->*ExecutePtr)();
        }

        bool CanExecute(void* obj) const override
        {
            if (!ExecutePtr) return false;
            if (!CanExecutePtr) return true;
            const T* typedObj = static_cast<const T*>(obj);
            return (typedObj->*CanExecutePtr)();
        }

        void Execute(World& world, void* obj) const override
        {
            PHX_ASSERT(false);
        }

        bool CanExecute(const World& world, void* obj) const override
        {
            PHX_ASSERT(false);
            return false;
        }

        TExecutePtr ExecutePtr = nullptr;
        TCanExecutePtr CanExecutePtr = nullptr;
    };

    struct PHOENIX_SIM_API StaticFunctionPointer : IMethodPointer
    {
        using TExecutePtr = void(*)();
        using TCanExecutePtr = bool(*)();

        StaticFunctionPointer(TExecutePtr executePtr, TCanExecutePtr canExecutePtr = nullptr)
            : ExecutePtr(executePtr)
            , CanExecutePtr(canExecutePtr)
        {
        }

        bool IsStatic() const override
        {
            return true;
        }

        bool RequiresWorld() const override
        {
            return false;
        }

        void Execute(void* obj) const override
        {
            PHX_ASSERT(obj == nullptr);
            PHX_ASSERT(CanExecute(obj));
            ExecutePtr();
        }

        bool CanExecute(void* obj) const override
        {
            PHX_ASSERT(obj == nullptr);
            if (!ExecutePtr) return false;
            return !CanExecutePtr || CanExecutePtr();
        }

        void Execute(World& world, void* obj) const override
        {
            PHX_ASSERT(false);
        }

        bool CanExecute(const World& world, void* obj) const override
        {
            PHX_ASSERT(false);
            return false;
        }

        TExecutePtr ExecutePtr = nullptr;
        TCanExecutePtr CanExecutePtr = nullptr;
    };

    struct PHOENIX_SIM_API StaticWorldFunctionPointer : IMethodPointer
    {
        using TExecutePtr = void(*)(World& world);
        using TCanExecutePtr = bool(*)(const World& world);

        StaticWorldFunctionPointer(TExecutePtr executePtr, TCanExecutePtr canExecutePtr = nullptr)
            : ExecutePtr(executePtr)
            , CanExecutePtr(canExecutePtr)
        {
        }

        bool IsStatic() const override
        {
            return true;
        }

        bool RequiresWorld() const override
        {
            return true;
        }

        void Execute(void* obj) const override
        {
            PHX_ASSERT(false);
        }

        bool CanExecute(void* obj) const override
        {
            PHX_ASSERT(false);
            return false;
        }

        void Execute(World& world, void* obj) const override
        {
            PHX_ASSERT(obj == nullptr);
            PHX_ASSERT(CanExecute(obj));
            ExecutePtr(world);
        }

        bool CanExecute(const World& world, void* obj) const override
        {
            PHX_ASSERT(obj == nullptr);
            if (!ExecutePtr) return false;
            return !CanExecutePtr || CanExecutePtr(world);
        }

        TExecutePtr ExecutePtr = nullptr;
        TCanExecutePtr CanExecutePtr = nullptr;
    };

    struct PHOENIX_SIM_API IPropertyAccessor
    {
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

    struct PHOENIX_SIM_API PropertyDescriptor : DescriptorBase
    {
        EPropertyValueType ValueType = EPropertyValueType::Unknown;
        std::shared_ptr<IPropertyAccessor> PropertyAccessor;
    };

    template <class T, class TValue>
    struct PropertyAccessor : IPropertyAccessor
    {
        using TGetter = TValue(T::*)() const;
        using TSetter = void(T::*)(const TValue&);

        PropertyAccessor(TGetter getter, TSetter setter = nullptr) : Getter(getter), Setter(setter) {}

        TValue Get(const void* obj) const
        {
            PHX_ASSERT(obj);
            PHX_ASSERT(Getter);
            const T* typedObj = static_cast<const T*>(obj);
            return (typedObj->*Getter)();
        }

        void Set(void* obj, const TValue& val) const
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

    template <class TValue>
    struct StaticPropertyAccessor : IPropertyAccessor
    {
        using TGetter = TValue(*)();
        using TSetter = void(*)(const TValue&);

        StaticPropertyAccessor(TGetter getter, TSetter setter = nullptr) : Getter(getter), Setter(setter) {}

        TValue Get() const
        {
            PHX_ASSERT(Getter);
            return (*Getter)();
        }

        void Set(const TValue& val) const
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

    template <class T, class TValue>
    struct FieldAccessor : IPropertyAccessor
    {
        using TFieldPtr = TValue T::*;

        FieldAccessor(TFieldPtr ptr) : FieldPtr(ptr) {}

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

        TFieldPtr FieldPtr = nullptr;
    };

    template <class TValue>
    struct StaticFieldAccessor : IPropertyAccessor
    {
        using TFieldPtr = TValue*;

        StaticFieldAccessor(TFieldPtr ptr) : FieldPtr(ptr) {}

        const TValue& Get() const
        {
            PHX_ASSERT(FieldPtr);
            return *FieldPtr;
        }

        void Set(const TValue& val) const
        {
            PHX_ASSERT(FieldPtr);
            *FieldPtr = val;
        }

        bool IsReadOnly() const override
        {
            return false;
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

        TFieldPtr FieldPtr = nullptr;
    };

    template <class T>
    struct PropertyDescriptorBuilder
    {
        static EPropertyValueType GetPropertyValueType()
        {
#define TYPE_TO_ENUM_VALUE(type, enum_value) \
            if constexpr (std::is_same_v<T, type>) \
            { \
                return EPropertyValueType::enum_value; \
            }

            TYPE_TO_ENUM_VALUE(int8, Int8)
            TYPE_TO_ENUM_VALUE(uint8, UInt8)
            TYPE_TO_ENUM_VALUE(int16, Int16)
            TYPE_TO_ENUM_VALUE(uint16, UInt16)
            TYPE_TO_ENUM_VALUE(int32, Int32)
            TYPE_TO_ENUM_VALUE(uint32, UInt32)
            TYPE_TO_ENUM_VALUE(int64, Int64)
            TYPE_TO_ENUM_VALUE(uint64, UInt64)
            TYPE_TO_ENUM_VALUE(float, Float)
            TYPE_TO_ENUM_VALUE(double, Double)
            TYPE_TO_ENUM_VALUE(bool, Bool)
            TYPE_TO_ENUM_VALUE(std::string, String)
            TYPE_TO_ENUM_VALUE(FName, Name)
            TYPE_TO_ENUM_VALUE(Phoenix::Vec2, Vec2)

#undef TYPE_TO_ENUM_VALUE

            return EPropertyValueType::Unknown;
        }

        static std::unordered_map<std::string, std::string> GetMetadata()
        {
            return {};
        }
    };

    template <uint8 Tb, class T>
    struct PropertyDescriptorBuilder<TFixed<Tb, T>>
    {
        static EPropertyValueType GetPropertyValueType()
        {
            return EPropertyValueType::FixedPoint;
        }

        static std::unordered_map<std::string, std::string> GetMetadata()
        {
            return { { "FractionalBits", std::format("{}", Tb) } };
        }
    };

    struct PHOENIX_SIM_API BaseDescriptor
    {
        const char* CName;
        const struct TypeDescriptor* Descriptor; 
    };

    struct PHOENIX_SIM_API TypeDescriptor
    {
        virtual ~TypeDescriptor() = default;

        const char* GetCName() const { return CName; }
        FName GetFName() const { return FName; }
        std::string GetDisplayName() const { return DisplayName; }
        size_t GetSize() const { return Size; }

        template <class T>
        void RegisterBase()
        {
            const auto& baseDescriptor = T::GetStaticTypeDescriptor();
            BaseDescriptor& descriptor = Bases[baseDescriptor.GetCName()];
            descriptor.CName = baseDescriptor.GetCName();
            descriptor.Descriptor = &baseDescriptor;
        }

        template <class T, class TValue>
        const PropertyDescriptor& RegisterProperty(
            const std::string& name,
            typename PropertyAccessor<T, TValue>::TGetter getter,
            typename PropertyAccessor<T, TValue>::TSetter setter = nullptr)
        {
            PropertyDescriptor& descriptor = Properties[name];
            descriptor.Name = name;
            descriptor.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            descriptor.Metadata = PropertyDescriptorBuilder<TValue>::GetMetadata();
            descriptor.PropertyAccessor = std::make_shared<PropertyAccessor<T, TValue>>(getter, setter);
            return descriptor;
        }

        template <class TValue>
        const PropertyDescriptor& RegisterProperty(
            const std::string& name,
            typename StaticPropertyAccessor<TValue>::TGetter getter,
            typename StaticPropertyAccessor<TValue>::TSetter setter = nullptr)
        {
            PropertyDescriptor& descriptor = Properties[name];
            descriptor.Name = name;
            descriptor.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            descriptor.Metadata = PropertyDescriptorBuilder<TValue>::GetMetadata();
            descriptor.PropertyAccessor = std::make_shared<StaticPropertyAccessor<TValue>>(getter, setter);
            return descriptor;
        }

        template <class TValue>
        const PropertyDescriptor& RegisterProperty(
            const std::string& name,
            typename StaticWorldPropertyAccessor<TValue>::TGetter getter,
            typename StaticWorldPropertyAccessor<TValue>::TSetter setter = nullptr)
        {
            PropertyDescriptor& descriptor = Properties[name];
            descriptor.Name = name;
            descriptor.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            descriptor.Metadata = PropertyDescriptorBuilder<TValue>::GetMetadata();
            descriptor.PropertyAccessor = std::make_shared<StaticWorldPropertyAccessor<TValue>>(getter, setter);
            return descriptor;
        }

        template <class T, class TValue>
        const PropertyDescriptor& RegisterProperty(
            const std::string& name,
            typename FieldAccessor<T, TValue>::TFieldPtr fieldPtr)
        {
            PropertyDescriptor& descriptor = Properties[name];
            descriptor.Name = name;
            descriptor.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            descriptor.Metadata = PropertyDescriptorBuilder<TValue>::GetMetadata();
            descriptor.PropertyAccessor = std::make_shared<FieldAccessor<T, TValue>>(fieldPtr);
            return descriptor;
        }

        template <class TValue>
        const PropertyDescriptor& RegisterProperty(
            const std::string& name,
            typename StaticFieldAccessor<TValue>::TFieldPtr fieldPtr)
        {
            PropertyDescriptor& descriptor = Properties[name];
            descriptor.Name = name;
            descriptor.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            descriptor.Metadata = PropertyDescriptorBuilder<TValue>::GetMetadata();
            descriptor.PropertyAccessor = std::make_shared<StaticFieldAccessor<TValue>>(fieldPtr);
            return descriptor;
        }

        template <class T>
        const MethodDescriptor& RegisterMethod(
            const std::string& name,
            typename MethodPointer<T>::TExecutePtr executePtr,
            typename MethodPointer<T>::TCanExecutePtr canExecutePtr = nullptr)
        {
            MethodDescriptor& descriptor = Methods[name];
            descriptor.Name = name;
            descriptor.MethodPointer = std::make_shared<MethodPointer<T>>(executePtr, canExecutePtr);
            return descriptor;
        }

        template <class T>
        const MethodDescriptor& RegisterConstMethod(
            const std::string& name,
            typename ConstMethodPointer<T>::TExecutePtr executePtr,
            typename ConstMethodPointer<T>::TCanExecutePtr canExecutePtr = nullptr)
        {
            MethodDescriptor& descriptor = Methods[name];
            descriptor.Name = name;
            descriptor.MethodPointer = std::make_shared<ConstMethodPointer<T>>(executePtr, canExecutePtr);
            return descriptor;
        }

        const MethodDescriptor& RegisterStaticMethod(
            const std::string& name,
            StaticFunctionPointer::TExecutePtr executePtr,
            StaticFunctionPointer::TCanExecutePtr canExecutePtr = nullptr)
        {
            MethodDescriptor& descriptor = Methods[name];
            descriptor.Name = name;
            descriptor.MethodPointer = std::make_shared<StaticFunctionPointer>(executePtr, canExecutePtr);
            return descriptor;
        }

        const MethodDescriptor& RegisterStaticMethod(
            const std::string& name,
            StaticWorldFunctionPointer::TExecutePtr executePtr,
            StaticWorldFunctionPointer::TCanExecutePtr canExecutePtr = nullptr)
        {
            MethodDescriptor& descriptor = Methods[name];
            descriptor.Name = name;
            descriptor.MethodPointer = std::make_shared<StaticWorldFunctionPointer>(executePtr, canExecutePtr);
            return descriptor;
        }

        bool IsInterface() const
        {
            return HasAnyFlags(Flags, ETypeDescriptorFlags::Interface);
        }

        void DefaultConstruct(void* data) const
        {
            if (DefaultConstructFunc)
            {
                DefaultConstructFunc(data);
            }
        }

        void Destruct(void* data) const
        {
            if (DestructFunc)
            {
                DestructFunc(data);
            }
        }

        bool IsA(const FName& baseTypeId) const
        {
            for (const BaseDescriptor& baseDescriptor : Bases | std::ranges::views::values)
            {
                if (baseDescriptor.Descriptor->FName == baseTypeId)
                {
                    return true;
                }
                return baseDescriptor.Descriptor->IsA(baseTypeId);
            }
            return false;
        }

        template <class TBase>
        bool IsA() const
        {
            return IsA(TBase::StaticTypeName);
        }

        template <class TCallback>
        void ForEachInterface(const TCallback& callback) const
        {
            for (const BaseDescriptor& baseDescriptor : Bases | std::ranges::views::values)
            {
                if (baseDescriptor.Descriptor->IsInterface())
                {
                    InvokeForEachCallbackNoIndex(callback, *baseDescriptor.Descriptor);
                }

                baseDescriptor.Descriptor->ForEachInterface(callback);
            }
        }

        std::unordered_map<std::string, PropertyDescriptor> Properties;
        std::unordered_map<std::string, MethodDescriptor> Methods;
        std::unordered_map<std::string, BaseDescriptor> Bases;
        void (*DefaultConstructFunc)(void*) = nullptr;
        void (*DestructFunc)(void*) = nullptr;
        const char* CName;
        FName FName;
        const char* DisplayName;
        size_t Size;
        ETypeDescriptorFlags Flags = ETypeDescriptorFlags::None;
    };

    template <class TClass>
    bool IsA(const FName& baseTypeId)
    {
        return TClass::GetStaticTypeDescriptor().IsA(baseTypeId);
    }

    template <class TClass, class TBase>
    bool IsA()
    {
        return TClass::GetStaticTypeDescriptor().template IsA<TBase>();
    }

    template <class TBase, class TClass>
    bool IsA(const TClass* ptr)
    {
        return ptr->GetTypeDescriptor().template IsA<TBase>();
    }

    template <class TBase, class TClass>
    bool IsA(const std::shared_ptr<TClass>& ptr)
    {
        return ptr->GetTypeDescriptor().template IsA<TBase>();
    }

    template <class TBase, class TClass>
    TBase* Cast(TClass* ptr)
    {
        return IsA<TBase>(ptr) ? static_cast<TBase*>(ptr) : nullptr;
    }

    template <class TBase, class TClass>
    const TBase* Cast(const TClass* ptr)
    {
        return IsA<TBase>(ptr) ? static_cast<const TBase*>(ptr) : nullptr;
    }

    template <class TBase, class TClass>
    std::shared_ptr<TBase> Cast(const std::shared_ptr<TClass>& ptr)
    {
        return IsA<TBase>(ptr) ? std::static_pointer_cast<TBase>(ptr) : nullptr;
    }

    template <class TBase, class TClass>
    std::shared_ptr<const TBase> Cast(const std::shared_ptr<const TClass>& ptr)
    {
        return IsA<TBase>(ptr) ? std::static_pointer_cast<const TBase>(ptr) : nullptr;
    }

    template <class T, class T2 = void>
    struct TTypeHelper;

    template <class T>
    struct TTypeHelper<T, std::enable_if_t<!std::is_default_constructible_v<T>>>
    {
        static void DefaultConstruct(void* data)
        {
            PHX_ASSERT(false);
            // memset(data, 0, sizeof(T));
        }
        static void Destruct(void* data)
        {
            static_cast<T*>(data)->~T();
        }
    };

    template <class T>
    struct TTypeHelper<T, std::enable_if_t<std::is_default_constructible_v<T>>>
    {
        static void DefaultConstruct(void* data) { new (data) T(); }
        static void Destruct(void* data) { static_cast<T*>(data)->~T(); }
    };

    template <class>
    struct TTypeDescriptor
    {
        using Type = TypeDescriptor;
    };

    template <class T>
    concept IsValidBaseType = requires
    {
        T::GetStaticTypeDescriptor();
    };

#define PHX_DECLARE_TYPE_BEGIN_(type, base, flags) \
    public: \
        using ThisType = type; \
        using BaseType = base; \
    private: \
        struct STypeDescriptor { \
            static constexpr Phoenix::FName StaticFName = #type##_n; \
            static constexpr const char* StaticCName = #type; \
            static auto Construct() \
            { \
                Phoenix::TypeDescriptor descriptor; \
                descriptor.Flags = flags; \
                descriptor.Size = sizeof(type); \
                descriptor.DefaultConstructFunc = &Phoenix::TTypeHelper<type>::DefaultConstruct; \
                descriptor.DestructFunc = &Phoenix::TTypeHelper<type>::Destruct; \
                descriptor.FName = type::STypeDescriptor::StaticFName; \
                descriptor.CName = type::STypeDescriptor::StaticCName; \
                descriptor.DisplayName = type::STypeDescriptor::StaticCName; \
                if constexpr ( Phoenix::IsValidBaseType<base> ) \
                { \
                    descriptor.RegisterBase<base>(); \
                }

#define PHX_DECLARE_TYPE_END_() \
                return descriptor; \
            } \
            static const auto& StaticGet() \
            { \
                static Phoenix::TypeDescriptor definition = Construct(); \
                return definition; \
            } \
        }; \
    public: \
        static constexpr Phoenix::FName StaticTypeName = STypeDescriptor::StaticFName; \
        static const Phoenix::TypeDescriptor& GetStaticTypeDescriptor() { return STypeDescriptor::StaticGet(); } \

//
// Declare type with base
//

#define PHX_DECLARE_TYPE_WITH_BASE_BEGIN(type, base) PHX_DECLARE_TYPE_BEGIN_(type, base, Phoenix::ETypeDescriptorFlags::None)
#define PHX_DECLARE_TYPE_WITH_BASE_END() \
    PHX_DECLARE_TYPE_END_() \
    const Phoenix::TypeDescriptor& GetTypeDescriptor() const override { return GetStaticTypeDescriptor(); } \

#define PHX_DECLARE_TYPE_WITH_BASE(type, base) \
    PHX_DECLARE_TYPE_WITH_BASE_BEGIN(type, base) \
    PHX_DECLARE_TYPE_WITH_BASE_END()

//
// Declare type
//

#define PHX_DECLARE_TYPE_BEGIN(type) PHX_DECLARE_TYPE_BEGIN_(type, void, Phoenix::ETypeDescriptorFlags::None)
#define PHX_DECLARE_TYPE_END() \
    PHX_DECLARE_TYPE_END_() \
    virtual const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return GetStaticTypeDescriptor(); } \

#define PHX_DECLARE_TYPE(type) \
    PHX_DECLARE_TYPE_BEGIN(type) \
    PHX_DECLARE_TYPE_END()

//
// Declare interface with base
//

#define PHX_DECLARE_INTERFACE_WITH_BASE_BEGIN(type, base) PHX_DECLARE_TYPE_BEGIN_(type, base, ETypeDescriptorFlags::Interface)
#define PHX_DECLARE_INTERFACE_WITH_BASE_END() PHX_DECLARE_TYPE_WITH_BASE_END()

#define PHX_DECLARE_INTERFACE_WITH_BASE(type, base) \
    PHX_DECLARE_INTERFACE_WITH_BASE_BEGIN(type, base) \
    PHX_DECLARE_INTERFACE_WITH_BASE_END()

//
// Declare interface
//

#define PHX_DECLARE_INTERFACE_BEGIN(type) \
    public: \
        virtual ~type() = default; \
    PHX_DECLARE_INTERFACE_WITH_BASE_BEGIN(type, void)

#define PHX_DECLARE_INTERFACE_END() PHX_DECLARE_TYPE_END()

#define PHX_DECLARE_INTERFACE(type) \
    PHX_DECLARE_INTERFACE_BEGIN(type) \
    PHX_DECLARE_INTERFACE_END()

//
// Member registration
//

#define PHX_REGISTER_FIELD(type, name) descriptor.RegisterProperty<ThisType, type>(#name, &ThisType::name);
#define PHX_REGISTER_STATIC_FIELD(type, name) descriptor.RegisterProperty<type>(#name, &ThisType::name);
#define PHX_REGISTER_PROPERTY(type, name) descriptor.RegisterProperty<ThisType, type>(#name, &ThisType::Get##name, &ThisType::Set##name);
#define PHX_REGISTER_STATIC_PROPERTY(type, name) descriptor.RegisterProperty<type>(#name, &ThisType::Get##name, &ThisType::Set##name);
#define PHX_REGISTER_METHOD(name) descriptor.RegisterMethod<ThisType>(#name, &ThisType::name);
#define PHX_REGISTER_CONST_METHOD(name) descriptor.RegisterConstMethod<ThisType>(#name, &ThisType::name);
#define PHX_REGISTER_STATIC_METHOD(name) descriptor.RegisterStaticMethod(#name, &ThisType::name);
#define PHX_REGISTER_BASE(name) descriptor.RegisterBase<name>();
}
