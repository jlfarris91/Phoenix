#include "ModelDescriptor.h"

#include "PropertyDescriptor.h"

namespace Phoenix::PropertyGrid
{

std::string ModelDescriptor::GetId() const
{
    return Id;
}

void ModelDescriptor::SetId(const std::string& id)
{
    Id = id;
}

std::string ModelDescriptor::GetDisplayName() const
{
    return DisplayName.GetValue(Id);
}

void ModelDescriptor::SetDisplayName(const Phoenix::Attribute<std::string>& displayName)
{
    DisplayName = displayName;
}

std::string ModelDescriptor::GetDescription() const
{
    return Description.GetValue();
}

void ModelDescriptor::SetDescription(const Phoenix::Attribute<std::string>& description)
{
    Description = description;
}

const std::shared_ptr<IModelDescriptor>& ModelDescriptor::GetParent() const
{
    return Parent;
}

void ModelDescriptor::SetParent(const std::shared_ptr<IModelDescriptor>& parent)
{
    Parent = parent;
}

const std::vector<std::shared_ptr<Phoenix::IObject>>& ModelDescriptor::GetTargets() const
{
    return Targets;
}

void ModelDescriptor::SetTargets(const std::vector<std::shared_ptr<Phoenix::IObject>>& targets)
{
    Targets = targets;
}

const std::vector<std::shared_ptr<IPropertyDescriptor>>& ModelDescriptor::GetProperties() const
{
    return Properties;
}

std::shared_ptr<IPropertyDescriptor> ModelDescriptor::FindProperty(const std::string& id) const
{
    for (const auto& property : Properties)
    {
        if (property->GetId() == id)
        {
            return property;
        }
    }
    return {};
}

bool ModelDescriptor::SendMessage(type_id typeId, const void* data)
{
    if (AcceptMessage(typeId, data))
    {
        return true;
    }

    return Parent && Parent->SendMessage(typeId, data);
}

} // namespace Phoenix::PropertyGrid
