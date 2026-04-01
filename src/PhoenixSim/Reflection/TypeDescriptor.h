#pragma once

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Reflection/FieldDescriptor.h"
#include "PhoenixSim/Reflection/MethodDescriptor.h"
#include "PhoenixSim/Reflection/PropertyDescriptor.h"
#include "PhoenixSim/Reflection/TypeName.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API ETypeDescriptorFlags : uint8
    {
        None = 0,
        Class = 1,
        Interface = 2,
        ScriptHidden = 4,
        NoScriptTable = 8,  // opt-out of Lua metatable generation for this type
    };

    class PHOENIX_SIM_API TypeDescriptor
    {
    public:

        FTypeName GetTypeName() const;

        FName GetTypeId() const;

        const std::string& GetName() const;

        const std::string& GetAlias() const;

        const std::string& GetAliasOrName() const;

        const std::string& GetQualifiedName() const;

        const std::string& GetDisplayName() const;

        uint32 GetSize() const;

        const std::unordered_set<FName>& GetBaseTypeIds() const;

        const std::unordered_map<std::string, FieldDescriptor>& GetFields() const;

        const std::unordered_map<std::string, PropertyDescriptor>& GetProperties() const;

        const std::unordered_map<std::string, MethodDescriptor>& GetMethods()    const;

        uint32 GetAllMembers(std::vector<const MemberDescriptor*>& outMembers) const;

        const std::unordered_map<std::string, std::string>& GetMetadata()   const;

        const std::vector<MethodDescriptor>& GetConstructors() const;

        const MethodDescriptor& GetDestructor()   const;

        ETypeDescriptorFlags GetFlags() const;

        bool IsClass() const;

        // Returns true if the type is complex, meaning it has fields, properties, or methods.
        bool IsComplex() const;

        bool IsInterface() const;

        bool IsNoScriptTable()  const;

        bool IsScriptHidden()   const;

        bool IsA(const FName& baseTypeId) const;

        template <class TBase>
        bool IsA() const
        {
            return IsA(StaticTypeName<TBase>::TypeId);
        }

        bool IsTemplate(const char* templateName) const;

        void DefaultConstruct(void* data) const;

        void Destruct(void* data) const;

    private:

        friend class TypeRegistry;
        template <class T> friend class TypeDescriptorBuilder;
        template <class T> friend class ScriptRegistrationBuilder;

        FTypeName               TypeName;
        FName                   TypeId;
        std::string             QualifiedName;
        std::string             Name;
        std::string             DisplayName;
        std::string             Alias;
        uint32                  Size = 0;
        ETypeDescriptorFlags    Flags = ETypeDescriptorFlags::None;

        std::unordered_map<std::string, FieldDescriptor>    Fields;
        std::unordered_map<std::string, PropertyDescriptor> Properties;
        std::unordered_map<std::string, MethodDescriptor>   Methods;
        std::unordered_set<FName>                           Bases;
        std::vector<MethodDescriptor>                       Constructors;
        MethodDescriptor                                    Destructor;
        std::unordered_map<std::string, std::string>        Metadata;
    };

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
}
