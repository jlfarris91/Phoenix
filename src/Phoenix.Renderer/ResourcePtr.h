#pragma once

#include "ResourceHandle.h"
#include "Phoenix/Name.h"
#include "Phoenix/Reflection/TypeDescriptor.h"

namespace Phoenix::Renderer
{
    class IResource;
    class IResourceManager;

    class ResourcePtr
    {
    public:
        ResourcePtr() = default;
        ResourcePtr(FName fullName);

        const IResource* LoadResource(IResourceManager& manager);
        const IResource* LoadResource(IResourceManager& manager) const;

        const IResource* GetResource(IResourceManager& manager);
        const IResource* GetResource(IResourceManager& manager) const;

    private:
        FName FullName;
        HResource CachedHandle;
    };

    template <class T>
    class TResourcePtr : public ResourcePtr
    {
    public:
        TResourcePtr() = default;
        TResourcePtr(FName fullName) : ResourcePtr(fullName) {}

        const T* LoadResource(IResourceManager& manager)
        {
            return Cast<T>(ResourcePtr::LoadResource(manager));
        }

        const T* LoadResource(IResourceManager& manager) const
        {
            return Cast<T>(ResourcePtr::LoadResource(manager));
        }

        const T* GetResource(IResourceManager& manager)
        {
            return Cast<T>(ResourcePtr::GetResource(manager));
        }

        const T* GetResource(IResourceManager& manager) const
        {
            return Cast<T>(ResourcePtr::GetResource(manager));
        }
    };
}
