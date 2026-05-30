#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <glm/vec2.hpp>
#include <SDL3/SDL.h>
#include "IResourceManager.h"
#include "SDL3PlatformService.h"

namespace Phoenix::Renderer
{
    class IResourceLoader;
}

namespace Phoenix::App::Dev
{
    class LineMesh2D;

    class SDL3ResourceManager : public Renderer::IResourceManager
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3ResourceManager, Phoenix::Renderer::IResourceManager)
    public:
        using Dependencies = std::tuple<SDL3PlatformService>;
        explicit SDL3ResourceManager(std::shared_ptr<SDL3PlatformService> platform);
        ~SDL3ResourceManager() override;

        void Initialize(const std::shared_ptr<Application>& application) override;

        Renderer::HResource StoreResource(std::unique_ptr<Renderer::IResource> resource, FName name = {}) override;

        Renderer::HResource LoadResource(const std::string& path) override;

        size_t LoadResources(const std::string& path, std::vector<Renderer::HResource>& outHandles) override;

        const Renderer::IResource* GetResourceRaw(Renderer::HResource handle) const override;

        const Renderer::IResource* FindResourceRaw(FName name) const override;

        bool ReleaseResource(Renderer::HResource handle) override;

        Renderer::HResource CreateRenderTarget(uint32_t width, uint32_t height);

    private:

        std::shared_ptr<SDL3PlatformService> Platform;
        std::unordered_map<uint32_t, std::unique_ptr<Renderer::IResource>> Resources;
        std::vector<std::shared_ptr<Renderer::IResourceLoader>> ResourceLoaders;
        uint32_t NextResourceId = 0;
    };
}
