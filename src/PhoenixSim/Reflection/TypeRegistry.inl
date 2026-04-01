#pragma once

#include "TypeRegistry.h"
#include "Reflection.h"

namespace Phoenix
{
    template <class T, class ... TBases>
    TypeDescriptor& TypeRegistry::GetOrCreate()
    {
        constexpr const char*   typeCName = StaticTypeName<T>::GetName();
        constexpr FName         typeFName = FName(typeCName);

        auto& map = GetMap();
        const auto key = static_cast<hash32_t>(typeFName);
        auto [it, inserted] = map.emplace(key, nullptr);
        if (inserted)
        {
            it->second = std::make_unique<TypeDescriptor>();
            TypeDescriptor& desc = *it->second;
            desc.Size           = sizeof(T);
            desc.TypeName       = StaticTypeName<T>::Get();
            desc.FName          = typeFName;
            desc.DisplayName    = typeCName;

            // Register bases
            (desc.RegisterBase<TBases>(), ...);   // idempotent (map key prevents duplicates)

            // Default constructor — only registered if T is default-constructible.
            // HasSelfParam=true: 'self' is pre-allocated memory to construct into.
            if constexpr (std::is_default_constructible_v<T>)
            {
                MethodDescriptor ctor;
                ctor.Name = "__construct__";
                ctor.Return.Type = MakeGenericValueTypeRef<void>();
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
                dtor.Return.Type = MakeGenericValueTypeRef<void>();
                dtor.Function.HasSelfParam = true;
                dtor.Function.Invoke = [](void* obj, std::span<const GenericValue>) {
                    static_cast<T*>(obj)->~T();
                    return GenericValue::Void();
                };
                desc.Destructor = std::move(dtor);
            }

            desc.Metadata = TypeDescriptorMetadataProvider<T>::GetMetadata();
        }

        return *it->second;
    }
}