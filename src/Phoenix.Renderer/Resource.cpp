#include "Resource.h"

Phoenix::Renderer::HResource Phoenix::Renderer::IResource::GetHandle() const
{
    return { Id, GetResourceType() };
}

Phoenix::FName Phoenix::Renderer::IResource::GetFullName() const
{
    return FullName;
}

void Phoenix::Renderer::ResourceInit::Init(IResource& resource, uint32_t id, FName fullName)
{
    resource.Id = id;
    resource.FullName = fullName;

    if (FName::IsNoneOrEmpty(resource.FullName))
    {
        resource.FullName = std::format("{}_{}", resource.GetTypeDescriptor().GetQualifiedName(), id);
    }
}
