#include "Reflection.h"

using namespace Phoenix;

const char* TypeDescriptor::GetCName() const
{
    return CName;
}

const char* TypeDescriptor::GetQualifiedCName() const
{
    return QualifiedCName ? QualifiedCName : CName;
}

FName TypeDescriptor::GetFName() const
{
    return FName;
}

const char* TypeDescriptor::GetDisplayName() const
{
    return DisplayName;
}

size_t TypeDescriptor::GetSize() const
{
    return Size;
}

const std::unordered_map<std::string, PropertyDescriptor>& TypeDescriptor::GetProperties() const
{
    return Properties;
}

const std::unordered_map<std::string, MethodDescriptor>& TypeDescriptor::GetMethods() const
{
    return Methods;
}

const std::unordered_map<std::string, std::string>& TypeDescriptor::GetMetadata() const
{
    return Metadata;
}

const std::vector<MethodDescriptor>& TypeDescriptor::GetConstructors() const
{
    return Constructors;
}

const MethodDescriptor& TypeDescriptor::GetDestructor() const
{
    return Destructor;
}

bool TypeDescriptor::IsInterface() const
{
    return HasAnyFlags(Flags, ETypeDescriptorFlags::Interface);
}

void TypeDescriptor::DefaultConstruct(void* data) const
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

void TypeDescriptor::Destruct(void* data) const
{
    if (Destructor.Function.Invoke)
    {
        Destructor.Execute(data);
    }
}

bool TypeDescriptor::IsA(const Phoenix::FName& baseTypeId) const
{
    for (const BaseDescriptor& base : Bases | std::ranges::views::values)
    {
        if (base.Descriptor->FName == baseTypeId)
        {
            return true;
        }
        if (base.Descriptor->IsA(baseTypeId))
        {
            return true;
        }
    }
    return false;
}

MethodDescriptor& TypeDescriptor::RegisterStaticMethod(
    const std::string& name,
    void(*executePtr)(),
    bool(*canExecutePtr)(),
    EMemberDescriptorFlags flags)
{
    MethodDescriptor d = MakeMethodDescriptor(name.c_str(), executePtr);
    d.Flags = flags;
    if (canExecutePtr)
    {
        d.CanExecutePredicate = [canExecutePtr](void*) { return canExecutePtr(); };
    }
    Methods[name] = std::move(d);
    return Methods[name];
}
