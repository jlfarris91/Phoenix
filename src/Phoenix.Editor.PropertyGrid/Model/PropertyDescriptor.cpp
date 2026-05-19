#include "PropertyDescriptor.h"

#include <algorithm>
#include "ModelDescriptor.h"

namespace Phoenix::PropertyGrid
{

std::string PropertyDescriptor::GetId() const
{
    return Id;
}

void PropertyDescriptor::SetId(const std::string& id)
{
    Id = id;
}

std::string PropertyDescriptor::GetDisplayName() const
{
    return DisplayName.GetValue(Id);
}

void PropertyDescriptor::SetDisplayName(const Phoenix::Attribute<std::string>& name)
{
    DisplayName = name;
}

std::string PropertyDescriptor::GetDescription() const
{
    return Description.GetValue();
}

void PropertyDescriptor::SetDescription(const std::string& description)
{
    Description = description;
}

std::string PropertyDescriptor::GetType() const
{
    return Type;
}

void PropertyDescriptor::SetType(const std::string& type)
{
    Type = type;
}

bool PropertyDescriptor::IsArray() const
{
    return bIsArray;
}

void PropertyDescriptor::SetIsArray(bool isArray)
{
    bIsArray = isArray;
}

int32_t PropertyDescriptor::GetArrayIndex() const
{
    return ArrayIndex;
}

void PropertyDescriptor::SetArrayIndex(int32_t index)
{
    ArrayIndex = index;
}

std::shared_ptr<IModelDescriptor> PropertyDescriptor::GetModelDescriptor() const
{
    return Model;
}

void PropertyDescriptor::SetModelDescriptor(const std::shared_ptr<ModelDescriptor>& model)
{
    if (Model)
    {
        (void)std::ranges::remove(Model->Properties, shared_from_this());
    }

    Model = model;

    if (Model)
    {
        Model->Properties.push_back(shared_from_this());
    }
}

std::shared_ptr<IPropertyDescriptor> PropertyDescriptor::GetParentProperty() const
{
    return Parent;
}

void PropertyDescriptor::SetParentProperty(const std::shared_ptr<PropertyDescriptor>& parent)
{
    if (Parent)
    {
        (void)std::ranges::remove(Parent->ChildProperties, shared_from_this());
    }

    Parent = parent;

    if (Parent)
    {
        Parent->ChildProperties.push_back(shared_from_this());
    }
}

const std::vector<std::shared_ptr<IPropertyDescriptor>>& PropertyDescriptor::GetChildProperties() const
{
    return ChildProperties;
}

PropertySetValueEvent& PropertyDescriptor::OnPropertySetEvent()
{
    return PropertySetValueEvent;
}

bool PropertyDescriptor::SendMessage(type_id typeId, const void* data)
{
    if (AcceptMessage(typeId, data))
    {
        return true;
    }

    if (auto modelDescriptor = GetModelDescriptor())
    {
        if (modelDescriptor->SendMessage(typeId, data))
        {
            return true;
        }
    }

    if (auto parentDescriptor = GetParentProperty())
    {
        return parentDescriptor->SendMessage(typeId, data);
    }

    return false;
}

} // namespace Phoenix::PropertyGrid
