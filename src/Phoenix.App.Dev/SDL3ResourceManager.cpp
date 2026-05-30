#include "SDL3ResourceManager.h"

#include <filesystem>
#include <fstream>

#include "ResourceLoader.h"
#include "Resources/RenderTarget2D.h"

using namespace Phoenix::App::Dev;
using namespace Phoenix::Renderer;

SDL3ResourceManager::SDL3ResourceManager(std::shared_ptr<SDL3PlatformService> platform)
    : Platform(std::move(platform))
{
}

SDL3ResourceManager::~SDL3ResourceManager()
{
    for (auto& [id, entry] : Resources)
    {
        entry->ReleaseResources();
    }
    Resources.clear();
}

HResource SDL3ResourceManager::StoreResource(std::unique_ptr<Renderer::IResource> resource)
{
    uint32_t resourceId = ++NextResourceId;
    resource->SetId(resourceId);
    Resources.emplace(resourceId, std::move(resource));
    return { resourceId, resource->GetResourceType() };
}

HResource SDL3ResourceManager::LoadResource(const char* path)
{
    std::vector<Renderer::HResource> handles;
    LoadResources(path, handles);
    return handles.empty() ? Renderer::HResource() : handles.front();
}

size_t SDL3ResourceManager::LoadResources(const char* path, std::vector<Renderer::HResource>& outHandles)
{
    std::ifstream stream(path);
    if (!stream.is_open())
    {
        return 0;
    }

    auto ext = std::filesystem::path(path).extension().string();

    Renderer::ResourceLoadArgs args = {
        .PlatformService = *Platform.get(),
        .FilePath = path,
        .Stream = stream
    };

    Renderer::IResourceLoader* foundLoader = nullptr;
    for (auto&& loader : ResourceLoaders)
    {
        if (loader->CanLoad(args))
        {
            foundLoader = loader.get();
        }
    }

    if (!foundLoader)
    {
        return 0;
    }

    std::vector<std::unique_ptr<Renderer::IResource>> loadedResources;
    if (!foundLoader->Load(args, loadedResources))
    {
        return 0;
    }

    for (auto&& resource : loadedResources)
    {
        auto handle = StoreResource(std::move(resource));
        outHandles.push_back(handle);
    }

    return loadedResources.size();
}

const IResource* SDL3ResourceManager::GetResource(Renderer::HResource handle) const
{
    auto iter = Resources.find(handle.Id);
    return iter != Resources.end() ? iter->second.get() : nullptr;
}

bool SDL3ResourceManager::ReleaseResource(Renderer::HResource handle)
{
    auto iter = Resources.find(handle.Id);
    if (iter == Resources.end())
    {
        return false;
    }

    Resources.erase(iter);
    return true;
}

HResource SDL3ResourceManager::CreateRenderTarget(glm::vec2 size)
{
    SDL_Texture* texture = SDL_CreateTexture(
        Platform->GetRenderer(),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        size.x, size.y);
    auto resource = std::make_unique<RenderTarget2D>(texture, size);
    return StoreResource(std::move(resource));
}