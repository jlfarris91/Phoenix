#pragma once

#include "Resource.h"
#include "ResourceHandle.h"
#include "Phoenix.App/Application/AppService.h"

namespace Phoenix::Renderer
{
    class IResourceManager : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(IResourceManager, Phoenix::IAppService)
    public:

        virtual HResource           StoreResource(std::unique_ptr<IResource> resource, FName name = {}) = 0;
        virtual HResource           LoadResource(const std::string& path) = 0;
        virtual size_t              LoadResources(const std::string& path, std::vector<HResource>& outHandles) = 0;
        virtual const IResource*    GetResourceRaw(HResource handle) const = 0;
        virtual const IResource*    FindResourceRaw(FName name) const = 0;
        virtual bool                ReleaseResource(HResource handle) = 0;

        template <class T>
        const T* GetResource(HResource handle) const
        {
            return Cast<T>(GetResourceRaw(handle));
        }
    };
}
