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

        virtual HResource           StoreResource(std::unique_ptr<IResource> resource) = 0;
        virtual HResource           LoadResource(const char* path) = 0;
        virtual size_t              LoadResources(const char* path, std::vector<HResource>& outHandles) = 0;
        virtual const IResource*    GetResource(HResource handle) const = 0;
        virtual bool                ReleaseResource(HResource handle) = 0;
    };
}
