#pragma once

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    // ── TypeDescriptorBuilder<T> ────────────────────────────────────────────
    //
    // Fluent builder used inside PHX_TYPE_REGISTRATION blocks.
    // Writes property, method, and base metadata directly into the type's
    // TypeDescriptor (obtained via TypeRegistry::GetOrCreate<T>()).
    //
    // Usage:
    //
    //   PHX_TYPE_REGISTRATION(TransformComponent)
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

    private:
        MethodDescriptor* Descriptor = nullptr;
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
            auto& propertyDescriptor = Descriptor->RegisterProperty<T, TValue>(name, ptr);
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

        TypeDescriptorBuilder& Method(
            const char* name,
            void(T::* fn)(),
            bool(T::* canFn)() const,
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto& methodDescriptor = Descriptor->RegisterMethod<T>(name, fn, canFn);
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&methodDescriptor);
                method(methodBuilder);
            }
            return *this;
        }

        TypeDescriptorBuilder& Method(
            const char* name,
            void(T::* fn)(),
            const MethodDescriptorBuilderFunc& method = {})
        {
            return Method(name, fn, nullptr, method);
        }

        TypeDescriptorBuilder& Method(
            const char* name,
            void(T::* fn)() const,
            bool(T::* canFn)() const,
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto& methodDescriptor = Descriptor->RegisterConstMethod<T>(name, fn, canFn);
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&methodDescriptor);
                method(methodBuilder);
            }
            return *this;
        }

        TypeDescriptorBuilder& Method(
            const char* name,
            void(T::* fn)() const,
            const MethodDescriptorBuilderFunc& method = {})
        {
            return Method(name, fn, nullptr, nullptr, method);
        }

        TypeDescriptorBuilder& StaticMethod(
            const char* name,
            void(*fn)(),
            bool(*canFn)(),
            const MethodDescriptorBuilderFunc& method = {})
        {
            auto& methodDescriptor = Descriptor->RegisterStaticMethod(name, fn, canFn);
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&methodDescriptor);
                method(methodBuilder);
            }
            return *this;
        }

        TypeDescriptorBuilder& StaticMethod(
            const char* name,
            void(*fn)(),
            const MethodDescriptorBuilderFunc& method = {})
        {
            return StaticMethod(name, fn, nullptr, method);
        }

        // ── Type-level metadata ───────────────────────────────────────────────

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

    // ── PHX_TYPE_REGISTRATION ─────────────────────────────────────────────────
    //
    // Places registration code in a static initializer that runs before main(). The body has access to a pre-bound
    // 'registration' TypeDescriptorBuilder<Type>.
    //
    // Also defines Type::GetStaticTypeDescriptor() — the non-inline exported
    // symbol that forces the linker to pull this TU in.
    //
    //   PHX_TYPE_REGISTRATION(TransformComponent)
    //   {
    //       registration
    //           .Field("Transform", &TransformComponent::Transform);
    //   }

#define PHX_TYPE_REGISTRATION(Type)                                                      \
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
