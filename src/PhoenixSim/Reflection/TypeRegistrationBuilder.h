#pragma once

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    // ── TypeRegistrationBuilder<T> ────────────────────────────────────────────
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

    template <class T>
    class TypeRegistrationBuilder
    {
    public:
        explicit TypeRegistrationBuilder()
            : m_desc(&TypeRegistry::GetOrCreate<T>())
        {}

        // ── Fields (direct member pointer) ───────────────────────────────────

        template <class TValue>
        TypeRegistrationBuilder& Field(const char* name, TValue T::* ptr)
        {
            m_desc->RegisterProperty<T, TValue>(name, ptr);
            return *this;
        }

        // ── Properties (getter / setter) ─────────────────────────────────────

        template <class TValue>
        TypeRegistrationBuilder& Property(
            const char* name,
            TValue (T::* getter)() const,
            void (T::* setter)(const TValue&) = nullptr)
        {
            m_desc->RegisterProperty<T, TValue>(name, getter, setter);
            return *this;
        }

        // ── Methods ───────────────────────────────────────────────────────────

        TypeRegistrationBuilder& Method(
            const char* name,
            void(T::* fn)(),
            bool(T::* canFn)() const = nullptr)
        {
            m_desc->RegisterMethod<T>(name, fn, canFn);
            return *this;
        }

        TypeRegistrationBuilder& Method(
            const char* name,
            void(T::* fn)() const,
            bool(T::* canFn)() const = nullptr)
        {
            m_desc->RegisterConstMethod<T>(name, fn, canFn);
            return *this;
        }

        TypeRegistrationBuilder& StaticMethod(
            const char* name,
            void(*fn)(),
            bool(*canFn)() = nullptr)
        {
            m_desc->RegisterStaticMethod(name, fn, canFn);
            return *this;
        }

        // ── Additional bases ─────────────────────────────────────────────────

        template <class TBase>
        TypeRegistrationBuilder& Base()
        {
            m_desc->RegisterBase<TBase>();
            return *this;
        }

    private:
        TypeDescriptor* m_desc = nullptr;
    };

    // ── PHX_TYPE_REGISTRATION ─────────────────────────────────────────────────
    //
    // Places registration code in a static initializer that runs before main(). The body has access to a pre-bound
    // 'registration' TypeRegistrationBuilder<Type>.
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
        Phoenix::TypeRegistrationBuilder<Type>& registration);                           \
    namespace {                                                                          \
        struct _PhxTypeReg_##Type##_t                                                    \
        {                                                                                \
            _PhxTypeReg_##Type##_t() noexcept                                            \
            {                                                                            \
                Phoenix::TypeRegistrationBuilder<Type> registration;                     \
                _PhxTypeReg_##Type##_body(registration);                                 \
            }                                                                            \
        } _phx_type_reg_##Type##_instance;                                               \
    }                                                                                    \
    static void _PhxTypeReg_##Type##_body(                                               \
        Phoenix::TypeRegistrationBuilder<Type>& registration)

} // namespace Phoenix
