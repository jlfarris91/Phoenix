#include "BaseDescriptor.h"

const std::string& Phoenix::BaseDescriptor::GetName() const
{
    return Name;
}

const std::string& Phoenix::BaseDescriptor::GetDisplayName() const
{
    return DisplayName.empty() ? GetName() : DisplayName;
}

void Phoenix::BaseDescriptor::SetDisplayName(const std::string& displayName)
{
    DisplayName = displayName;
}

const std::unordered_map<std::string, std::string>& Phoenix::BaseDescriptor::GetMetadata() const
{
    return Metadata;
}
