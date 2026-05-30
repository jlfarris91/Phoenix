#include "ResourcePtr.h"

#include "IResourceManager.h"

Phoenix::Renderer::ResourcePtr::ResourcePtr(FName fullName)
    : FullName(fullName)
{
}

const Phoenix::Renderer::IResource* Phoenix::Renderer::ResourcePtr::LoadResource(IResourceManager& manager)
{
    if (auto resource = GetResource(manager))
    {
        return resource;
    }

    auto handle = manager.LoadResource(FullName.ToString());
    if (!handle.IsValid())
    {
        return nullptr;
    }

    CachedHandle = handle;
    return manager.GetResourceRaw(handle);
}

const Phoenix::Renderer::IResource* Phoenix::Renderer::ResourcePtr::LoadResource(IResourceManager& manager) const
{
    if (auto resource = GetResource(manager))
    {
        return resource;
    }

    auto handle = manager.LoadResource(FullName.ToString());
    if (!handle.IsValid())
    {
        return nullptr;
    }

    return manager.GetResourceRaw(handle);
}

const Phoenix::Renderer::IResource* Phoenix::Renderer::ResourcePtr::GetResource(IResourceManager& manager)
{
    if (auto resource = manager.GetResourceRaw(CachedHandle))
    {
        return resource;
    }

    if (auto resource = manager.FindResourceRaw(FullName))
    {
        CachedHandle = resource->GetHandle();
        return resource;
    }

    return nullptr;
}

const Phoenix::Renderer::IResource* Phoenix::Renderer::ResourcePtr::GetResource(IResourceManager& manager) const
{
    if (auto resource = manager.GetResourceRaw(CachedHandle))
    {
        return resource;
    }

    if (auto resource = manager.FindResourceRaw(FullName))
    {
        return resource;
    }

    return nullptr;
}
