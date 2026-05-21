#pragma once

#include <memory>
#include <optional>

#include "Phoenix/Name.h"
#include "Phoenix/Reflection/EnumValueDescriptor.h"
#include "Phoenix/Reflection/FieldDescriptor.h"
#include "Phoenix/Reflection/MethodDescriptor.h"
#include "Phoenix/Reflection/PropertyDescriptor.h"
#include "Phoenix/Reflection/TypeName.h"

namespace Phoenix
{
    enum class PHOENIX_SIM_API ETypeDescriptorFlags : uint8
    {
        None            = 0,
        Class           = 1,
        Enum            = 2,
        EnumFlags       = 4,
        Interface       = 8,
        ScriptHidden    = 16
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

        // Returns the dot-separated script namespace path for this type.
        // Explicit "Namespace" metadata takes priority; otherwise derived from
        // FTypeName::GetScriptPath() using the alias (if any) for the local name.
        // e.g. TVec2<TFixed<12,int>> alias="Vec2"  →  "Phoenix.Vec2"
        //      Phoenix::RTS::FeatureUnit            →  "Phoenix.RTS.FeatureUnit"
        std::string GetScriptNamespace() const;

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

        bool IsScriptHidden()   const;

        bool IsA(const FName& baseTypeId) const;

        template <class TBase>
        bool IsA() const
        {
            return IsA(StaticTypeName<TBase>::TypeId);
        }

        bool IsTemplate(const char* templateName) const;

        bool CanDefaultConstruct() const;

        void DefaultConstruct(void* data) const;

        void Destruct(void* data) const;

        std::shared_ptr<void> MakeShared() const;

        std::string ToString(const void* obj) const;

        // ── Enums ───────────────────────────────────────────────────────────

        bool IsEnum() const;
        bool IsEnumFlags() const;

        FName GetEnumUnderlyingTypeId() const;

        const TypeDescriptor* GetEnumUnderlyingType() const;

        const std::vector<EnumValueDescriptor>& GetEnumValues() const;

        const EnumValueDescriptor* GetEnumValueDescriptor(uint32 index) const;
        const EnumValueDescriptor* GetEnumValueDescriptor(const std::string& name) const;
        const EnumValueDescriptor* GetEnumValueDescriptor(const Variant& value) const;
        const EnumValueDescriptor* GetEnumValueDescriptor(const void* value) const;

        const Variant* GetEnumValue(uint32 index) const;
        const Variant* GetEnumValue(const std::string& name) const;

        template <class TValue>
        std::optional<TValue> GetEnumValueAs(uint32 index) const
        {
            auto value = GetEnumValue(index);
            return value ? value->As<TValue>() : std::optional<TValue>();
        }

        template <class TValue>
        std::optional<TValue> GetEnumValueAs(const std::string& name) const
        {
            auto value = GetEnumValue(name);
            return value ? value->As<TValue>() : std::optional<TValue>();
        }

        Variant ConstructEnumValue(const std::unordered_set<uint32>& flagIndices) const;

        bool HasEnumFlag(const void* value, const void* flag) const;
        void SetEnumFlag(void* value, const void* flag) const;
        void ClearEnumFlag(void* value, const void* flag) const;

    private:

        friend class TypeRegistry;
        template <class T> friend class TypeDescriptorBuilder;
        template <class T> friend class EnumDescriptorBuilder;
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
        std::function<std::string(const void*)>             ToStringFunc;

        // Enum specific fields
        FName                               EnumUnderlyingTypeId;
        std::vector<EnumValueDescriptor>    EnumValues;
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
