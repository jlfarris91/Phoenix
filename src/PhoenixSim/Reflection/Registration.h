#pragma once

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    // ── TypeDescriptorBuilder<T> ────────────────────────────────────────────
    //
    // Fluent builder used inside PHX_DEFINE_TYPE blocks.
    // Writes property, method, and base metadata directly into the type's
    // TypeDescriptor (obtained via TypeRegistry::GetOrCreate<T>()).
    //
    // Usage:
    //
    //   PHX_DEFINE_TYPE(TransformComponent)
    //   {
    //       registration
    //           .Field("AttachParent", &TransformComponent::AttachParent)
    //           .Field("Transform",    &TransformComponent::Transform);
    //   }

    template <class TBuilder>
    class DescriptorBaseBuilder
    {
    public:
        DescriptorBaseBuilder(DescriptorBase* descriptor)
            : Descriptor(descriptor)
        {}

        TBuilder& Metadata(const std::string& key, const std::string& value)
        {
            Descriptor->Metadata[key] = value;
            return *reinterpret_cast<TBuilder*>(this);
        }

        template <class TValue>
        TBuilder& Metadata(const std::string& key, const TValue& value)
        {
            Descriptor->Metadata[key] = std::to_string(value);
            return *reinterpret_cast<TBuilder*>(this);
        }

        TBuilder& Category(const std::string& name)
        {
            return Metadata("Category", name);
        }

        TBuilder& SortOrder(int32 order)
        {
            return Metadata("SortOrder", order);
        }

        TBuilder& DisplayName(const std::string& name)
        {
            return Metadata("DisplayName", name);
        }

        TBuilder& ScriptHidden()
        {
            SetFlagRef(Descriptor->Flags, EMemberDescriptorFlags::ScriptHidden);
            return *reinterpret_cast<TBuilder*>(this);
        }

        template <class ...TFlag>
        TBuilder& Flags(TFlag&&... flags)
        {
            (SetFlagRef(Descriptor->Flags, flags), ...);
            return *reinterpret_cast<TBuilder*>(this);
        }

    private:
        DescriptorBase* Descriptor = nullptr;
    };
    
    template <class TBuilder>
    class PropertyDescriptorBuilderBase : public DescriptorBaseBuilder<TBuilder>
    {
    public:
        PropertyDescriptorBuilderBase(PropertyDescriptor* descriptor)
            : DescriptorBaseBuilder<TBuilder>(descriptor)
            , Descriptor(descriptor)
        {}

    private:
        PropertyDescriptor* Descriptor = nullptr;
    };

    template <class TValue>
    class PropertyDescriptorBuilder : public PropertyDescriptorBuilderBase<PropertyDescriptorBuilder<TValue>>
    {
        using Super = PropertyDescriptorBuilderBase<PropertyDescriptorBuilder>;
    public:

        PropertyDescriptorBuilder(PropertyDescriptor* descriptor)
            : Super(descriptor)
        {}
    };

    template <class T>
    concept IsNumerical = requires
    {
        (std::is_integral_v<T> || std::is_floating_point_v<T>) && !std::is_same_v<T, bool>;
    };

    template <IsNumerical TValue>
    class PropertyDescriptorBuilder<TValue> : public PropertyDescriptorBuilderBase<PropertyDescriptorBuilder<TValue>>
    {
        using Super = PropertyDescriptorBuilderBase<PropertyDescriptorBuilder>;
    public:
        PropertyDescriptorBuilder(PropertyDescriptor* descriptor)
            : Super(descriptor)
        {}

        PropertyDescriptorBuilder& MinValue(TValue value)
        {
            this->Metadata("MinValue", std::to_string(value));
            return *this;
        }

        PropertyDescriptorBuilder& MaxValue(TValue value)
        {
            this->Metadata("MaxValue", std::to_string(value));
            return *this;
        }

        PropertyDescriptorBuilder& Step(TValue value)
        {
            this->Metadata("Step", std::to_string(value));
            return *this;
        }
    };

    class MethodDescriptorBuilder : public DescriptorBaseBuilder<MethodDescriptorBuilder>
    {
    public:
        MethodDescriptorBuilder(MethodDescriptor* descriptor)
            : DescriptorBaseBuilder(descriptor)
        {}
    };

    template <class TValue>
    using PropertyDescriptorBuilderFunc = std::function<void(PropertyDescriptorBuilder<TValue>& builder)>;

    using MethodDescriptorBuilderFunc = std::function<void(MethodDescriptorBuilder& builder)>;

    template <class T>
    class TypeDescriptorBuilder
    {
    public:
        explicit TypeDescriptorBuilder()
            : Descriptor(&TypeRegistry::GetOrCreate<T>())
        {}

        // ── Fields (direct member pointer) ───────────────────────────────────

        template <class TValue>
        TypeDescriptorBuilder& Field(
            const char* name,
            TValue T::* ptr,
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& field = {})
        {
            auto& propertyDescriptor = Descriptor->RegisterField<T, TValue>(name, ptr);
            if (field)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&propertyDescriptor);
                field(propertyBuilder);
            }
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& StaticField(
            const char* name,
            TValue* ptr,
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& field = {})
        {
            auto& propertyDescriptor = Descriptor->RegisterField<TValue>(name, ptr);
            if (field)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&propertyDescriptor);
                field(propertyBuilder);
            }
            return *this;
        }

        // ── Properties (getter / setter) ─────────────────────────────────────

        template <class TValue>
        TypeDescriptorBuilder& Property(
            const char* name,
            TValue (T::*getter)() const,
            void (T::*setter)(const TValue&),
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            auto& propertyDescriptor = Descriptor->RegisterProperty<T, TValue>(name, getter, setter);
            if (property)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&propertyDescriptor);
                property(propertyBuilder);
            }
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& Property(
            const char* name,
            TValue (T::* getter)() const,
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            return Property(name, getter, nullptr, property);
        }

        // ── Methods ───────────────────────────────────────────────────────────

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& Method(
            const char* name,
            TRet(T::* fn)(TArgs...),
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto [iter, success] = Descriptor->Methods.emplace(name, MakeMethodDescriptor(name, fn));
            if (success && method)
            {
                MethodDescriptorBuilder methodBuilder(&iter->second);
                method(methodBuilder);
            }
            return *this;
        }

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& Method(
            const char* name,
            TRet(T::* fn)(TArgs...) const,
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto [iter, success] = Descriptor->Methods.emplace(name, MakeMethodDescriptor(name, fn));
            if (success && method)
            {
                MethodDescriptorBuilder methodBuilder(&iter->second);
                method(methodBuilder);
            }
            return *this;
        }

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& StaticMethod(
            const char* name,
            TRet(*fn)(TArgs...),
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto [iter, success] = Descriptor->Methods.emplace(name, MakeMethodDescriptor(name, fn));
            if (success && method)
            {
                MethodDescriptorBuilder methodBuilder(&iter->second);
                method(methodBuilder);
            }
            return *this;
        }

        // ── Type-level metadata ───────────────────────────────────────────────

        // Opt out of automatic Lua metatable generation for this type.
        TypeDescriptorBuilder& NoScriptTable()
        {
            SetFlagRef(Descriptor->Flags, ETypeDescriptorFlags::NoScriptTable);
            return *this;
        }

        // Dot-separated namespace used by scripting runtimes (e.g. "Phoenix.Unit").
        TypeDescriptorBuilder& Namespace(const char* ns)
        {
            Descriptor->Metadata["Namespace"] = ns;
            return *this;
        }

        TypeDescriptorBuilder& Metadata(const std::string& key, const std::string& value)
        {
            Descriptor->Metadata[key] = value;
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& Metadata(const std::string& key, const TValue& value)
        {
            return Metadata(key, std::to_string(value));
        }

        // ── Additional bases ─────────────────────────────────────────────────

        template <class TBase>
        TypeDescriptorBuilder& Base()
        {
            Descriptor->RegisterBase<TBase>();
            return *this;
        }

        template <class ...TBases>
        TypeDescriptorBuilder& Bases()
        {
            (Base<TBases>(), ...);
            return *this;
        }

    private:
        TypeDescriptor* Descriptor = nullptr;
    };

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
    // Use when no PHX_DEFINE_TYPE block is needed.
    // GetStaticTypeDescriptor() is defined inline — no .cpp required.
    //
    //   struct Command  { PHX_DECLARE_TYPE(Command) ... };
    //   class  IFeature { PHX_DECLARE_TYPE(IFeature, IService) ... };

#define PHX_DECLARE_TYPE(type, ...) \
    PHX_TYPE_BODY_(type) \
    private: \
        inline static const bool _s_phx_type_init_ = \
            (Phoenix::TypeRegistry::GetOrCreate<type, ##__VA_ARGS__>(), true); \
    public: \
        static const Phoenix::TypeDescriptor& GetStaticTypeDescriptor() \
        { \
            return Phoenix::TypeRegistry::GetOrCreate<type, ##__VA_ARGS__>(); \
        } \
        const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return GetStaticTypeDescriptor(); }

    // ── PHX_REFLECT_TYPE ──────────────────────────────────────────────────────
    //
    // Use when a PHX_DEFINE_TYPE block exists in a .cpp.
    // GetStaticTypeDescriptor() is declared here and *defined* by
    // PHX_DEFINE_TYPE — the non-inline definition forces the linker to
    // include the registration TU.
    //
    //   class FeatureECS : public IFeature { PHX_REFLECT_TYPE(FeatureECS, IFeature) ... };

#define PHX_REFLECT_TYPE(type, ...) \
    PHX_TYPE_BODY_(type) \
    private: \
        inline static const bool _s_phx_type_init_ = \
            (Phoenix::TypeRegistry::GetOrCreate<type, ##__VA_ARGS__>(), true); \
    public: \
        static const Phoenix::TypeDescriptor& GetStaticTypeDescriptor(); \
        const Phoenix::TypeDescriptor& GetTypeDescriptor() const { return GetStaticTypeDescriptor(); }

    // ── PHX_DEFINE_TYPE ─────────────────────────────────────────────────
    //
    // Places field/method registration code in a static initializer that runs
    // before main(). Also defines Type::GetStaticTypeDescriptor() — the
    // non-inline exported symbol that forces the linker to pull this TU in.
    //
    //   PHX_DEFINE_TYPE(TransformComponent)
    //   {
    //       registration
    //           .Field("Transform", &TransformComponent::Transform);
    //   }

#define PHX_DEFINE_TYPE(Type)                                                      \
    const Phoenix::TypeDescriptor& Type::GetStaticTypeDescriptor()                       \
    {                                                                                    \
        return Phoenix::TypeRegistry::GetOrCreate<Type>();                               \
    }                                                                                    \
    static void _PhxTypeReg_##Type##_body(                                               \
        Phoenix::TypeDescriptorBuilder<Type>& registration);                             \
    namespace {                                                                          \
        struct _PhxTypeReg_##Type##_t                                                    \
        {                                                                                \
            _PhxTypeReg_##Type##_t() noexcept                                            \
            {                                                                            \
                Phoenix::TypeDescriptorBuilder<Type> registration;                       \
                _PhxTypeReg_##Type##_body(registration);                                 \
            }                                                                            \
        } _phx_type_reg_##Type##_instance;                                               \
    }                                                                                    \
    static void _PhxTypeReg_##Type##_body(                                               \
        Phoenix::TypeDescriptorBuilder<Type>& registration)

} // namespace Phoenix
