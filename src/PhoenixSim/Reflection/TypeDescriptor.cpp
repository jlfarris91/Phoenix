#include "TypeDescriptor.h"

#include <ranges>

#include "TypeRegistry.h"
#include "PhoenixSim/Flags.h"

Phoenix::FTypeName Phoenix::TypeDescriptor::GetTypeName() const
{
    return TypeName;
}

Phoenix::FName Phoenix::TypeDescriptor::GetTypeId() const
{
    return TypeId;
}

const std::string& Phoenix::TypeDescriptor::GetName() const
{
    return Name;
}

const std::string& Phoenix::TypeDescriptor::GetAlias() const
{
    return Alias;
}

const std::string& Phoenix::TypeDescriptor::GetAliasOrName() const
{
    return Alias.empty() ? Name : Alias;
}

const std::string& Phoenix::TypeDescriptor::GetQualifiedName() const
{
    return QualifiedName;
}

// Builds the dot-separated script namespace path for this type.
//
//   alias        — caller-supplied alias (e.g. "Vec2" for TVec2<...>).
//                  If empty, falls back to TemplateName (for templates) or Name.
//
// Examples:
//   TVec2<TFixed<12,int>>, alias="Vec2"  →  "Phoenix.Vec2"
//   Phoenix::RTS::FeatureUnit, alias=""  →  "Phoenix.RTS.FeatureUnit"
std::string Phoenix::TypeDescriptor::GetScriptNamespace() const
{
    const auto it = Metadata.find("Namespace");
    if (it != Metadata.end())
        return it->second;

    // Convert the C++ parent namespace (e.g. "Phoenix::RTS") to dot notation.
    std::string parent = TypeName.GetParent();
    for (size_t i = 0; i + 1 < parent.size(); )
    {
        if (parent[i] == ':' && parent[i + 1] == ':') { parent.replace(i, 2, "."); }
        else ++i;
    }

    // Local name: alias > bare template name > short name.
    std::string local = !Alias.empty()         ? Alias
                      : (TypeName.GetNumTemplateArgs() > 0)  ? std::string(TypeName.GetTemplateName())
                                               : std::string(Name);

    return parent.empty() ? local : parent + "." + local;
}

const std::string& Phoenix::TypeDescriptor::GetDisplayName() const
{
    return DisplayName.empty() ? GetAliasOrName() : DisplayName;
}

Phoenix::uint32 Phoenix::TypeDescriptor::GetSize() const
{
    return Size;
}

const std::unordered_set<Phoenix::FName>& Phoenix::TypeDescriptor::GetBaseTypeIds() const
{
    return Bases;
}

const std::unordered_map<std::string, Phoenix::FieldDescriptor>& Phoenix::TypeDescriptor::GetFields() const
{
    return Fields;
}

const std::unordered_map<std::string, Phoenix::PropertyDescriptor>& Phoenix::TypeDescriptor::GetProperties() const
{
    return Properties;
}

const std::unordered_map<std::string, Phoenix::MethodDescriptor>& Phoenix::TypeDescriptor::GetMethods() const
{
    return Methods;
}

Phoenix::uint32 Phoenix::TypeDescriptor::GetAllMembers(std::vector<const MemberDescriptor*>& outMembers) const
{
    outMembers.reserve(Fields.size() + Properties.size() + Methods.size());
    uint32 count = 0;
    for (const auto& field : Fields | std::views::values)
    {
        outMembers.emplace_back(&field);
        ++count;
    }
    for (const auto& property : Properties | std::views::values)
    {
        outMembers.emplace_back(&property);
        ++count;
    }
    for (const auto& method : Methods | std::views::values)
    {
        outMembers.emplace_back(&method);
        ++count;
    }
    return count;
}

const std::unordered_map<std::string, std::string>& Phoenix::TypeDescriptor::GetMetadata() const
{
    return Metadata;
}

const std::vector<Phoenix::MethodDescriptor>& Phoenix::TypeDescriptor::GetConstructors() const
{
    return Constructors;
}

const Phoenix::MethodDescriptor& Phoenix::TypeDescriptor::GetDestructor() const
{
    return Destructor;
}

Phoenix::ETypeDescriptorFlags Phoenix::TypeDescriptor::GetFlags() const
{
    return Flags;
}

bool Phoenix::TypeDescriptor::IsClass() const
{
    return HasAnyFlags(Flags, ETypeDescriptorFlags::Class);
}

bool Phoenix::TypeDescriptor::IsComplex() const
{
    return !Fields.empty() || !Properties.empty() || !Methods.empty();
}

bool Phoenix::TypeDescriptor::IsInterface() const
{
    return HasAnyFlags(Flags, ETypeDescriptorFlags::Interface);
}

bool Phoenix::TypeDescriptor::IsNoScriptTable() const
{
    return HasAnyFlags(Flags, ETypeDescriptorFlags::NoScriptTable);
}

bool Phoenix::TypeDescriptor::IsScriptHidden() const
{
    return HasAnyFlags(Flags, ETypeDescriptorFlags::ScriptHidden);
}

bool Phoenix::TypeDescriptor::IsA(const FName& baseTypeId) const
{
    if (Bases.contains(baseTypeId))
    {
        return true;
    }
    for (const FName& base : Bases)
    {
        auto baseDescriptor = TypeRegistry::Get(base);
        if (baseDescriptor && baseDescriptor->IsA(baseTypeId))
        {
            return true;
        }
    }
    return false;
}

bool Phoenix::TypeDescriptor::IsTemplate(const char* templateName) const
{
    return FName(TypeName.GetTemplateName()) == FName(templateName);
}

void Phoenix::TypeDescriptor::DefaultConstruct(void* data) const
{
    for (const auto& ctor : Constructors)
    {
        if (ctor.GetParams().empty())
        {
            ctor.Execute(data);
            return;
        }
    }
}

void Phoenix::TypeDescriptor::Destruct(void* data) const
{
    Destructor.Execute(data);
}
