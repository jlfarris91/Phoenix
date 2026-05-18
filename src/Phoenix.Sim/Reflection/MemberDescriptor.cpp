#include "Phoenix.Sim/Reflection/MemberDescriptor.h"

#include "Phoenix.Sim/Flags.h"

const std::string& Phoenix::MemberDescriptor::GetCategory() const
{
    return Category;
}

void Phoenix::MemberDescriptor::SetCategory(const std::string& category)
{
    Category = category;
}

Phoenix::int32 Phoenix::MemberDescriptor::GetSortOrder() const
{
    return SortOrder;
}

void Phoenix::MemberDescriptor::SetSortOrder(int32 sortOrder)
{
    SortOrder = sortOrder;
}

Phoenix::EMemberDescriptorFlags Phoenix::MemberDescriptor::GetFlags() const
{
    return Flags;
}

bool Phoenix::MemberDescriptor::IsStatic() const
{
    return HasAnyFlags(Flags, EMemberDescriptorFlags::Static);
}

bool Phoenix::MemberDescriptor::IsReadOnly() const
{
    return HasAnyFlags(Flags, EMemberDescriptorFlags::ReadOnly);
}

bool Phoenix::MemberDescriptor::IsScriptHidden() const
{
    return HasAnyFlags(Flags, EMemberDescriptorFlags::ScriptHidden);
}
