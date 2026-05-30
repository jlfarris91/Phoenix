#pragma once

#include "Application/AppService.h"

namespace Phoenix::Renderer
{
    class IResource;

    struct ResourceLoadArgs
    {
        IPlatformService& PlatformService;
        std::string FilePath;
        std::istream& Stream;
    };

    class IResourceLoader : public IAppService
    {
        PHX_DECLARE_TYPE_DERIVED(IResourceLoader, IAppService)
    public:
        virtual bool CanLoad(const ResourceLoadArgs& args) const = 0;
        virtual size_t Load(const ResourceLoadArgs& args, std::vector<std::unique_ptr<IResource>>& outResources) = 0;
    };
}
