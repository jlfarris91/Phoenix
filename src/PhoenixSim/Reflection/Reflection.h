
#pragma once

#include <memory>
#include <ranges>

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Utils.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"
#include "PhoenixSim/Reflection/GenericValue.h"
#include "PhoenixSim/Reflection/GenericFunction.h"

namespace Phoenix
{
    class World;

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

    // ── GenericParam ──────────────────────────────────────────────────────────
    //
    // Describes one parameter (or the return value) of a MethodDescriptor.
    // Belongs to the reflection layer — GenericFunction itself carries no names.

    struct PHOENIX_SIM_API GenericParam
    {
        std::string  Name;
        ParamTypeRef Type;
        bool         IsOptional = false;
    };

    // ── MethodDescriptor ──────────────────────────────────────────────────────
    //
    // Full reflection descriptor for a method or function registered on a type.
    // Owns:
    //   • Params / Return  — parameter and return type metadata (for UI / docs /
    //                        marshaling by consumers such as LuaRuntime)
    //   • Function         — slim type-erased callable (GenericFunction)
    //   • CanExecutePredicate — optional runtime gate (e.g. CanStartAbility())
    //   • ScriptNamespace  — non-empty on functions registered via
    //                        PHX_SCRIPT_REGISTRATION; used by LuaRuntime to
    //                        determine which table to place the function in.
    //
    // Constructor convention:
    //   Entries in TypeDescriptor::Constructors use HasSelfParam = true with
    //   'self' pointing at pre-allocated memory.  Params describes only the
    //   user-visible constructor arguments, not the memory pointer.

    struct PHOENIX_SIM_API MethodDescriptor : DescriptorBase
    {
        std::vector<GenericParam> Params;
        GenericParam              Return;
        GenericFunction           Function;
        std::function<bool(void* self)> CanExecutePredicate;
        std::string               ScriptNamespace;

        bool IsStatic()   const { return !Function.HasSelfParam; }
        bool CanExecute(void* self) const { return !CanExecutePredicate || CanExecutePredicate(self); }
        GenericValue Execute(void* self, std::span<const GenericValue> args = {}) const { return Function.Invoke(self, args); }
    };

    // ── MakeMethodDescriptor ──────────────────────────────────────────────────
    //
    // Creates a fully-populated MethodDescriptor from a raw function pointer.
    // Fills Name, Params (with auto-generated positional names), Return, and
    // Function.  CanExecutePredicate and ScriptNamespace are left defaulted.

    namespace detail
    {
        template <class TRet, class... TArgs, size_t... I>
        void FillMethodParams(MethodDescriptor& d, std::index_sequence<I...>)
        {
            d.Params     = { GenericParam{ std::to_string(I), MakeParamTypeRef<TArgs>() }... };
            d.Return.Type = MakeParamTypeRef<TRet>();
        }
    }

    template <class TRet, class... TArgs>
    MethodDescriptor MakeMethodDescriptor(const char* name, TRet(*fn)(TArgs...))
    {
        MethodDescriptor d;
        d.Name     = name;
        d.Function = MakeGenericFunction(fn);
        detail::FillMethodParams<TRet, TArgs...>(d, std::index_sequence_for<TArgs...>{});
        return d;
    }

    template <class TClass, class TRet, class... TArgs>
    MethodDescriptor MakeMethodDescriptor(const char* name, TRet(TClass::*fn)(TArgs...))
    {
        MethodDescriptor d;
        d.Name     = name;
        d.Function = MakeGenericFunction(fn);
        detail::FillMethodParams<TRet, TArgs...>(d, std::index_sequence_for<TArgs...>{});
        return d;
    }

    template <class TClass, class TRet, class... TArgs>
    MethodDescriptor MakeMethodDescriptor(const char* name, TRet(TClass::*fn)(TArgs...) const)
    {
        MethodDescriptor d;
        d.Name     = name;
        d.Function = MakeGenericFunction(fn);
        detail::FillMethodParams<TRet, TArgs...>(d, std::index_sequence_for<TArgs...>{});
        return d;
    }

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

    struct PHOENIX_SIM_API BaseDescriptor
    {
        const char* CName = nullptr;
        const struct TypeDescriptor* Descriptor = nullptr;
    };

    struct PHOENIX_SIM_API TypeDescriptor
    {
        virtual ~TypeDescriptor() = default;

        // ── Public read-only interface ────────────────────────────────────────

        const char* GetCName() const { return CName; }
        Phoenix::FName GetFName() const { return FName; }
        const char* GetDisplayName() const { return DisplayName; }
        size_t GetSize() const { return Size; }

        const std::unordered_map<std::string, PropertyDescriptor>& GetProperties() const { return Properties; }
        const std::unordered_map<std::string, MethodDescriptor>&  GetMethods()    const { return Methods; }
        const std::vector<MethodDescriptor>&                       GetConstructors() const { return Constructors; }
        const MethodDescriptor&                                    GetDestructor()   const { return Destructor; }

        bool IsInterface() const { return HasAnyFlags(Flags, ETypeDescriptorFlags::Interface); }

        void DefaultConstruct(void* data) const
        {
            for (const auto& ctor : Constructors)
            {
                if (ctor.Params.empty())
                {
                    ctor.Execute(data);
                    return;
                }
            }
        }

        void Destruct(void* data) const
        {
            if (Destructor.Function.Invoke)
                Destructor.Execute(data);
        }

        bool IsA(const Phoenix::FName& baseTypeId) const
        {
            for (const BaseDescriptor& base : Bases | std::ranges::views::values)
            {
                if (base.Descriptor->FName == baseTypeId)
                    return true;
                return base.Descriptor->IsA(baseTypeId);
            }
            return false;
        }

        template <class TBase>
        bool IsA() const { return IsA(TBase::StaticTypeName); }

        template <class TCallback>
        void ForEachBaseClass(const TCallback& callback) const
        {
            for (const BaseDescriptor& base : Bases | std::ranges::views::values)
            {
                if (base.Descriptor)
                    InvokeForEachCallbackNoIndex(callback, *base.Descriptor);
                base.Descriptor->ForEachBaseClass(callback);
            }
        }

    private:
        // ── Mutation — accessible only to friends ─────────────────────────────

        template <class T>
        void RegisterBase()
        {
            const TypeDescriptor& baseDesc = T::GetStaticTypeDescriptor();
            BaseDescriptor& entry = Bases[baseDesc.GetCName()];
            entry.CName       = baseDesc.GetCName();
            entry.Descriptor  = &baseDesc;
        }

        template <class T, class TValue>
        void RegisterProperty(
            const std::string& name,
            TValue (T::* getter)() const,
            void (T::* setter)(const TValue&) = nullptr)
        {
            PropertyDescriptor& d = Properties[name];
            d.Name = name;
            d.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            d.Metadata  = PropertyDescriptorBuilder<TValue>::GetMetadata();
            d.PropertyAccessor = std::make_shared<PropertyAccessor<T, TValue>>(getter, setter);
        }

        template <class TValue>
        void RegisterProperty(
            const std::string& name,
            typename StaticPropertyAccessor<TValue>::TGetter getter,
            typename StaticPropertyAccessor<TValue>::TSetter setter = nullptr)
        {
            PropertyDescriptor& d = Properties[name];
            d.Name = name;
            d.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            d.Metadata  = PropertyDescriptorBuilder<TValue>::GetMetadata();
            d.PropertyAccessor = std::make_shared<StaticPropertyAccessor<TValue>>(getter, setter);
        }

        template <class TValue>
        void RegisterProperty(
            const std::string& name,
            typename StaticWorldPropertyAccessor<TValue>::TGetter getter,
            typename StaticWorldPropertyAccessor<TValue>::TSetter setter = nullptr)
        {
            PropertyDescriptor& d = Properties[name];
            d.Name = name;
            d.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            d.Metadata  = PropertyDescriptorBuilder<TValue>::GetMetadata();
            d.PropertyAccessor = std::make_shared<StaticWorldPropertyAccessor<TValue>>(getter, setter);
        }

        template <class T, class TValue>
        void RegisterProperty(const std::string& name, TValue T::* fieldPtr)
        {
            PropertyDescriptor& d = Properties[name];
            d.Name = name;
            d.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            d.Metadata  = PropertyDescriptorBuilder<TValue>::GetMetadata();
            d.PropertyAccessor = std::make_shared<FieldAccessor<T, TValue>>(fieldPtr);
        }

        template <class TValue>
        void RegisterProperty(const std::string& name, TValue* fieldPtr)
        {
            PropertyDescriptor& d = Properties[name];
            d.Name = name;
            d.ValueType = PropertyDescriptorBuilder<TValue>::GetPropertyValueType();
            d.Metadata  = PropertyDescriptorBuilder<TValue>::GetMetadata();
            d.PropertyAccessor = std::make_shared<StaticFieldAccessor<TValue>>(fieldPtr);
        }

        template <class T>
        void RegisterMethod(
            const std::string& name,
            void(T::* executePtr)(),
            bool(T::* canExecutePtr)() const = nullptr)
        {
            MethodDescriptor d = MakeMethodDescriptor(name.c_str(), executePtr);
            if (canExecutePtr)
                d.CanExecutePredicate = [canExecutePtr](void* self) {
                    return (static_cast<const T*>(self)->*canExecutePtr)();
                };
            Methods[name] = std::move(d);
        }

        template <class T>
        void RegisterConstMethod(
            const std::string& name,
            void(T::* executePtr)() const,
            bool(T::* canExecutePtr)() const = nullptr)
        {
            MethodDescriptor d = MakeMethodDescriptor(name.c_str(), executePtr);
            if (canExecutePtr)
                d.CanExecutePredicate = [canExecutePtr](void* self) {
                    return (static_cast<const T*>(self)->*canExecutePtr)();
                };
            Methods[name] = std::move(d);
        }

        void RegisterStaticMethod(
            const std::string& name,
            void(*executePtr)(),
            bool(*canExecutePtr)() = nullptr)
        {
            MethodDescriptor d = MakeMethodDescriptor(name.c_str(), executePtr);
            if (canExecutePtr)
                d.CanExecutePredicate = [canExecutePtr](void*) { return canExecutePtr(); };
            Methods[name] = std::move(d);
        }

        // ── Data ─────────────────────────────────────────────────────────────

        std::unordered_map<std::string, PropertyDescriptor> Properties;
        std::unordered_map<std::string, MethodDescriptor>   Methods;
        std::unordered_map<std::string, BaseDescriptor>     Bases;

        // Constructors (HasSelfParam=true, self=pre-allocated memory) and
        // Destructor registered automatically by TypeRegistry::GetOrCreate<T>.
        std::vector<MethodDescriptor> Constructors;
        MethodDescriptor              Destructor;

        const char*             CName       = nullptr;
        Phoenix::FName          FName       = {};
        const char*             DisplayName = nullptr;
        size_t                  Size        = 0;
        ETypeDescriptorFlags    Flags       = ETypeDescriptorFlags::None;

        // ── Friends ───────────────────────────────────────────────────────────
        friend class TypeRegistry;
        template <class T> friend class TypeRegistrationBuilder;
        template <class T> friend class ScriptRegistrationBuilder;
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

    // ── TypeRegistry::GetOrCreate — template definition ──────────────────────
    //
    // Defined here (after complete TypeDescriptor) even though declared in
    // TypeRegistry.h.  Any TU that includes Reflection.h gets this definition.

    template <class T, class... TBases>
    TypeDescriptor& TypeRegistry::GetOrCreate()
    {
        auto& map = GetMap();
        constexpr auto key = static_cast<hash32_t>(T::StaticTypeName);
        auto [it, inserted] = map.emplace(key, nullptr);
        if (inserted)
        {
            it->second = std::make_unique<TypeDescriptor>();
            TypeDescriptor& desc = *it->second;
            desc.Size        = sizeof(T);
            desc.CName       = T::StaticTypeCName;
            desc.FName       = T::StaticTypeName;
            desc.DisplayName = T::StaticTypeCName;

            // Default constructor — only registered if T is default-constructible.
            // HasSelfParam=true: 'self' is pre-allocated memory to construct into.
            if constexpr (std::is_default_constructible_v<T>)
            {
                MethodDescriptor ctor;
                ctor.Name = "__construct__";
                ctor.Return.Type = MakeParamTypeRef<void>();
                ctor.Function.HasSelfParam = true;
                ctor.Function.Invoke = [](void* mem, std::span<const GenericValue>) {
                    new(mem) T();
                    return GenericValue::Void();
                };
                desc.Constructors.push_back(std::move(ctor));
            }

            // Destructor.
            {
                MethodDescriptor dtor;
                dtor.Name = "__destruct__";
                dtor.Return.Type = MakeParamTypeRef<void>();
                dtor.Function.HasSelfParam = true;
                dtor.Function.Invoke = [](void* obj, std::span<const GenericValue>) {
                    static_cast<T*>(obj)->~T();
                    return GenericValue::Void();
                };
                desc.Destructor = std::move(dtor);
            }
        }
        TypeDescriptor& desc = *it->second;
        (desc.RegisterBase<TBases>(), ...);   // idempotent (map key prevents duplicates)
        return desc;
    }

    // ── PHX_TYPE_BODY_ ────────────────────────────────────────────────────────
    //
    // Emits the static type-name constants shared by both macros below.
    // No nested struct, no per-type Meyer singleton.

#define PHX_TYPE_BODY_(type) \
    public: \
        using ThisType = type; \
        static constexpr Phoenix::FName StaticTypeName  = #type##_n; \
        static constexpr const char*    StaticTypeCName = #type;

    // ── PHX_DECLARE_TYPE ──────────────────────────────────────────────────────
    //
    // Use when a PHX_TYPE_REGISTRATION block exists in a .cpp.
    // GetStaticTypeDescriptor() is declared here and *defined* by
    // PHX_TYPE_REGISTRATION — the non-inline definition is an exported symbol
    // that forces the linker to include the registration TU.
    //
    // The inline static _s_phx_type_init_ ensures bases are registered at
    // program start regardless of which TU first uses the type.
    //
    //   class FeatureECS : public IFeature {
    //       PHX_DECLARE_TYPE(FeatureECS, IFeature)
    //   };

#define PHX_DECLARE_TYPE(type, ...) \
    PHX_TYPE_BODY_(type) \
    private: \
        inline static const bool _s_phx_type_init_ = \
            (Phoenix::TypeRegistry::GetOrCreate<type, ##__VA_ARGS__>(), true); \
    public: \
        static const Phoenix::TypeDescriptor& GetStaticTypeDescriptor(); \
        virtual const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return GetStaticTypeDescriptor(); }

    // ── PHX_ENABLE_TYPE ───────────────────────────────────────────────────────
    //
    // Use when no PHX_TYPE_REGISTRATION block is needed (interfaces, simple
    // types).  GetStaticTypeDescriptor() is inline — no .cpp required.
    //
    //   class IFeature : public IService {
    //       PHX_ENABLE_TYPE(IFeature, IService)
    //   };

#define PHX_ENABLE_TYPE(type, ...) \
    PHX_TYPE_BODY_(type) \
    public: \
        static const Phoenix::TypeDescriptor& GetStaticTypeDescriptor() \
        { \
            return Phoenix::TypeRegistry::GetOrCreate<type, ##__VA_ARGS__>(); \
        } \
        virtual const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return GetStaticTypeDescriptor(); }

}
