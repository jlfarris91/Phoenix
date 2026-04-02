#pragma once

#include <ranges>
#include <array>

#include "MethodDescriptorString.h"
#include "PhoenixSim/Reflection/FieldAccessor.h"
#include "PhoenixSim/Reflection/FieldDescriptorBuilder.h"
#include "PhoenixSim/Reflection/MethodDescriptorBuilder.h"
#include "PhoenixSim/Reflection/PropertyAccessor.h"
#include "PhoenixSim/Reflection/PropertyDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeDescriptor.h"
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

    template <class TValue>
    using FieldDescriptorBuilderFunc = std::function<void(FieldDescriptorBuilder<TValue>& builder)>;

    template <class TValue>
    using PropertyDescriptorBuilderFunc = std::function<void(PropertyDescriptorBuilder<TValue>& builder)>;

    using MethodDescriptorBuilderFunc = std::function<void(MethodDescriptorBuilder& builder)>;

    template <class T>
    class TypeDescriptorBuilder
    {
    public:
        explicit TypeDescriptorBuilder()
            : Descriptor(&TypeRegistry::Get<T>())
        {}

        // ── Fields (direct member pointer) ───────────────────────────────────

        template <class TValue>
        TypeDescriptorBuilder& Field(
            const char* name,
            TValue T::* ptr,
            const std::type_identity_t<FieldDescriptorBuilderFunc<TValue>>& field = {})
        {
            FieldDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<FieldAccessor<T, TValue>>(ptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Field);
            if (field)
            {
                FieldDescriptorBuilder<TValue> fieldBuilder(&descriptor);
                field(fieldBuilder);
            }
            Descriptor->Fields[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& Field(
            const char* name,
            const TValue T::* ptr,
            const std::type_identity_t<FieldDescriptorBuilderFunc<TValue>>& field = {})
        {
            FieldDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<FieldAccessor<T, const TValue>>(ptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Field);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::ReadOnly);
            if (field)
            {
                FieldDescriptorBuilder<TValue> fieldBuilder(&descriptor);
                field(fieldBuilder);
            }
            Descriptor->Fields[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& StaticField(
            const char* name,
            TValue* ptr,
            const std::type_identity_t<FieldDescriptorBuilderFunc<TValue>>& field = {})
        {
            FieldDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<StaticFieldAccessor<TValue>>(ptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Field);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Static);
            if (field)
            {
                FieldDescriptorBuilder<TValue> fieldBuilder(&descriptor);
                field(fieldBuilder);
            }
            Descriptor->Fields[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& StaticField(
            const char* name,
            const TValue* ptr,
            const std::type_identity_t<FieldDescriptorBuilderFunc<TValue>>& field = {})
        {
            FieldDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<StaticFieldAccessor<const TValue>>(ptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Field);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Static);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::ReadOnly);
            if (field)
            {
                FieldDescriptorBuilder<TValue> fieldBuilder(&descriptor);
                field(fieldBuilder);
            }
            Descriptor->Fields[name] = std::move(descriptor);
            return *this;
        }

        // ── Properties (getter / setter) ─────────────────────────────────────

        template <class TValue, class TSetterValue>
        TypeDescriptorBuilder& Property(
            const char* name,
            TValue (T::*getter)() const,
            void (T::*setter)(TSetterValue),
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            PropertyDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<PropertyAccessor<T, TValue, TSetterValue>>(getter, setter);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Property);
            if (property)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&descriptor);
                property(propertyBuilder);
            }
            Descriptor->Properties[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& Property(
            const char* name,
            TValue (T::*getter)() const,
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            PropertyDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<PropertyAccessor<T, TValue>>(getter, nullptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Property);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::ReadOnly);
            if (property)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&descriptor);
                property(propertyBuilder);
            }
            Descriptor->Properties[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue, class TSetterValue>
        TypeDescriptorBuilder& StaticProperty(
            const char* name,
            TValue (*getter)(),
            void (*setter)(TSetterValue),
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            PropertyDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<StaticPropertyAccessor<TValue, TSetterValue>>(getter, setter);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Property);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Static);
            if (property)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&descriptor);
                property(propertyBuilder);
            }
            Descriptor->Properties[name] = std::move(descriptor);
            return *this;
        }

        template <class TValue>
        TypeDescriptorBuilder& StaticProperty(
            const char* name,
            TValue (*getter)(),
            const std::type_identity_t<PropertyDescriptorBuilderFunc<TValue>>& property = {})
        {
            PropertyDescriptor descriptor;
            descriptor.Name = name;
            descriptor.Type = &TypeRegistry::Get<TValue>();
            descriptor.Accessor = std::make_shared<StaticPropertyAccessor<TValue>>(getter, nullptr);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Property);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Static);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::ReadOnly);
            if (property)
            {
                PropertyDescriptorBuilder<TValue> propertyBuilder(&descriptor);
                property(propertyBuilder);
            }
            Descriptor->Properties[name] = std::move(descriptor);
            return *this;
        }

        // ── Methods ───────────────────────────────────────────────────────────

        template <class TRet, class ...TArgs, size_t ...I>
        static void FillMethodParams(
            MethodDescriptor& d,
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& declStr,
            std::index_sequence<I...>)
        {
            d.Name = std::string(declStr.Name);
            SetFlagRef(d.Flags, EMemberDescriptorFlags::Method);
            d.Params        = { ParamDescriptor{ std::string(declStr.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
            d.ReturnType    = &TypeRegistry::Get<TRet>();
        }

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& Method(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(T::* fn)(TArgs...),
            const MethodDescriptorBuilderFunc& method = {})
        {
            MethodDescriptor descriptor;
            descriptor.Function = MakeGenericFunction(fn);
            FillMethodParams<TRet, TArgs...>(descriptor, name, std::index_sequence_for<TArgs...>{});
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&descriptor);
                method(methodBuilder);
            }
            Descriptor->Methods[descriptor.Name] = std::move(descriptor);
            return *this;
        }

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& Method(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(T::* fn)(TArgs...) const,
            const MethodDescriptorBuilderFunc& method = {})
        {
            MethodDescriptor descriptor;
            descriptor.Function = MakeGenericFunction(fn);
            FillMethodParams<TRet, TArgs...>(descriptor, name, std::index_sequence_for<TArgs...>{});
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&descriptor);
                method(methodBuilder);
            }
            Descriptor->Methods[descriptor.Name] = std::move(descriptor);
            return *this;
        }

        template <class TRet, class... TArgs>
        TypeDescriptorBuilder& StaticMethod(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...),
            const MethodDescriptorBuilderFunc& method = {})
        {
            MethodDescriptor descriptor;
            descriptor.Function = MakeGenericFunction(fn);
            FillMethodParams<TRet, TArgs...>(descriptor, name, std::index_sequence_for<TArgs...>{});
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Static);
            if (method)
            {
                MethodDescriptorBuilder methodBuilder(&descriptor);
                method(methodBuilder);
            }
            Descriptor->Methods[descriptor.Name] = std::move(descriptor);
            return *this;
        }

        // ── Constructor ───────────────────────────────────────────────────────
        
        template <class ...TArgs>
        TypeDescriptorBuilder& Constructor(const MethodDescriptorBuilderFunc& constructor = {})
        {
            auto ctor = [](void* mem, TArgs&&... args) -> void
            {
                new(mem) T(std::forward<TArgs>(args)...);
            };

            MethodDeclarationString<TArgs...> declStr("__construct__");
            MethodDescriptor descriptor;
            descriptor.Function = MakeGenericFunctionTakingSelf(std::function(ctor));
            FillMethodParams<void, TArgs...>(descriptor, declStr, std::index_sequence_for<TArgs...>{});
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Method);
            SetFlagRef(descriptor.Flags, EMemberDescriptorFlags::Constructor);

            if (constructor)
            {
                MethodDescriptorBuilder methodBuilder(&descriptor);
                constructor(methodBuilder);
            }

            Descriptor->Constructors.emplace_back(std::move(descriptor));
            return *this;
        }

        // ── Type-level metadata ───────────────────────────────────────────────

        TypeDescriptorBuilder& Alias(const std::string& alias)
        {
            Descriptor->Alias = alias;
            return *this;
        }

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

        TypeDescriptorBuilder& Metadata(const std::string& key, const char* value)
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
            Descriptor->Bases.insert(StaticTypeName<TBase>::TypeId);
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
}
