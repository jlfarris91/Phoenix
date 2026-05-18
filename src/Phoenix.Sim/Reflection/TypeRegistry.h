#pragma once

#include <memory>
#include <unordered_map>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Utils.h"
#include "PhoenixSim/Reflection/GenericFunction.h"
#include "PhoenixSim/Reflection/TypeDescriptor.h"
#include "PhoenixSim/Reflection/TypeDescriptorMetadataProvider.h"
#include "PhoenixSim/Reflection/TypeName.h"

namespace Phoenix
{
    class TypeDescriptor;

    // Forward declaration to break circular dependency with TypeDescriptorBuilder.h.
    // Full definition is provided by Registration.h / TypeDescriptorBuilder.h at instantiation time.
    template <class T>
    class TypeDescriptorBuilder;

    template <class T>
    struct TypeRegistrar;

    template <class T>
    struct EnumRegistrar;

    // Forward declaration to break circular dependency with EnumDescriptorBuilder.h.
    // Full definition is provided by Registration.h / EnumDescriptorBuilder.h at instantiation time.
    template <class T>
    class EnumDescriptorBuilder;

    // Global registry that owns every reflected TypeDescriptor.
    // Types are created on first call to Get<T>() and live for the
    // lifetime of the program (Meyer-singleton storage inside GetMap()).
    class PHOENIX_SIM_API TypeRegistry
    {
    public:
        // Creates (or retrieves) the TypeDescriptor for T.
        // On first call the descriptor is constructed from T's static type info.
        // Subsequent calls are a no-op and just return the existing descriptor.
        template <class T>
        static TypeDescriptor& Get()
        {
            using TDecay = std::decay_t<T>;

            const FTypeName& typeName = StaticTypeName<TDecay>::TypeName;
            FName typeId = StaticTypeName<TDecay>::TypeId;

            auto& map = GetMap();
            auto [it, inserted] = map.emplace(typeId, nullptr);
            if (inserted)
            {
                it->second = std::make_unique<TypeDescriptor>();

                TypeDescriptor& desc = *it->second;
                desc.TypeName       = typeName;
                desc.TypeId         = typeId;
                desc.QualifiedName  = typeName.GetQualifiedName();
                desc.Name           = typeName.GetName();
                desc.DisplayName    = desc.Name;

                SetFlagRef(desc.Flags, ETypeDescriptorFlags::Class, std::is_class_v<TDecay>);
                SetFlagRef(desc.Flags, ETypeDescriptorFlags::Interface, std::is_abstract_v<TDecay>);

                if constexpr (std::is_enum_v<TDecay>)
                {
                    SetFlagRef(desc.Flags, ETypeDescriptorFlags::Enum);
                    desc.EnumUnderlyingTypeId = StaticTypeName<std::underlying_type_t<TDecay>>::TypeId;
                }

                if constexpr (!std::is_void_v<TDecay>)
                {
                    desc.Size = sizeof(TDecay);

                    // Default constructor — only registered if T is default-constructible.
                    if constexpr (std::is_default_constructible_v<TDecay>)
                    {
                        auto ctor = [](void* mem)
                        {
                            new(mem) TDecay();
                        };

                        MethodDescriptor descriptor;
                        descriptor.Name = "__construct__";
                        descriptor.ReturnType = &Get<void>();
                        descriptor.Function = MakeGenericFunctionTakingSelf(std::function(ctor));
                        desc.Constructors.push_back(std::move(descriptor));
                    }

                    // Destructor
                    if constexpr (std::is_destructible_v<TDecay>)
                    {
                        auto dtor = [](void* mem)
                        {
                            static_cast<TDecay*>(mem)->~TDecay();
                        };

                        MethodDescriptor descriptor;
                        descriptor.Name = "__destruct__";
                        descriptor.ReturnType = &Get<void>();
                        descriptor.Function = MakeGenericFunctionTakingSelf(std::function(dtor));
                        desc.Destructor = std::move(descriptor);
                    }

                    // ToString
                    if constexpr (requires { std::to_string(std::declval<TDecay>()); })
                    {
                        desc.ToStringFunc = [](const void* mem)
                        {
                            return std::to_string(*static_cast<const TDecay*>(mem));
                        };
                    }

                    // Extension point for custom metadata.
                    desc.Metadata = TypeDescriptorMetadataProvider<TDecay>::GetMetadata();

                    // Extension point for custom type registration logic.
                    // Only class types can have fields/methods registered via TypeDescriptorBuilder.
                    if constexpr (std::is_class_v<TDecay>)
                    {
                        TypeDescriptorBuilder<TDecay> builder;
                        TypeRegistrar<TDecay>::Register(builder);
                    }

                    // Extension point for custom enum registration logic.
                    if constexpr (std::is_enum_v<TDecay>)
                    {
                        EnumDescriptorBuilder<TDecay> builder;
                        EnumRegistrar<TDecay>::Register(builder);
                    }

                    // Also add a record for the alias, if one was defined.
                    // Use a separate non-owning map so the alias doesn't create a second
                    // unique_ptr owning the same object (which would cause a double-free
                    // when the static map is destroyed at program exit).
                    if (!desc.GetAlias().empty() && FName(desc.GetAlias()) != typeId)
                    {
                        GetAliasMap()[(hash32_t)FName(desc.GetAlias())] = &desc;
                    }
                }
            }

            // Re-look up by key rather than reusing `it`, because recursive Get<> calls
            // above may have triggered a map rehash that invalidates `it`.
            return *GetMap().at((hash32_t)typeId);
        }

        // Returns the descriptor for the given name, or nullptr if not registered.
        static const TypeDescriptor* Get(FName typeId);

        // Read-only view of all registered descriptors.
        static const std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& GetAll();

        // Returns all descriptors whose type derives from (or is) the given base type.
        // Only returns concrete (non-abstract) types.
        template <class T>
        static std::vector<const TypeDescriptor*> GetAllDerivedFrom()
        {
            const FName baseTypeId = StaticTypeName<T>::TypeId;
            std::vector<const TypeDescriptor*> result;
            for (const auto& [_, desc] : GetAll())
            {
                if (!desc || desc->IsInterface()) continue;
                if (desc->IsA(baseTypeId))
                    result.push_back(desc.get());
            }
            return result;
        }

        template <class TCallback>
        static void ForEachBaseClass(FName typeId, const TCallback& callback)
        {
            if (auto typeDescriptor = Get(typeId))
            {
                for (const FName& baseId : typeDescriptor->Bases)
                {
                    if (auto baseDescriptor = Get(baseId))
                    {
                        InvokeForEachCallbackNoIndex(callback, *baseDescriptor);
                        ForEachBaseClass(baseId, callback);
                    }
                }
            }
        }

        template <class T, class TCallback>
        static void ForEachBaseClass(const TCallback& callback)
        {
            for (const FName& baseId : Get<T>().Bases)
            {
                if (auto baseDescriptor = Get(baseId))
                {
                    InvokeForEachCallbackNoIndex(callback, *baseDescriptor);
                    ForEachBaseClass(baseId, callback);
                }
            }
        }

    private:
        static std::unordered_map<hash32_t, std::unique_ptr<TypeDescriptor>>& GetMap();

        // Non-owning map for alias lookups (alias name → existing descriptor).
        static std::unordered_map<hash32_t, TypeDescriptor*>& GetAliasMap();
    };
}
