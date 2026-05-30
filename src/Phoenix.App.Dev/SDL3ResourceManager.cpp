#include "SDL3ResourceManager.h"

#include <filesystem>
#include <fstream>
#include <ranges>

#include "ResourceLoader.h"
#include "Application/Application.h"
#include "Phoenix/Logging.h"
#include "Resources/Texture2D.h"

using namespace Phoenix::App::Dev;
using namespace Phoenix::Renderer;

namespace
{
    Phoenix::FName MakeResourceFullName(const std::string& path)
    {
        auto testPath = std::filesystem::path(path.c_str()).lexically_normal();
        if (testPath.is_absolute())
        {
            auto workingDir = std::filesystem::absolute("./Data/Catalogs/Core/Assets/");
            testPath = std::filesystem::relative(testPath, workingDir);
        }
        return testPath.generic_string();
    }
}

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

void SDL3ResourceManager::Initialize(const std::shared_ptr<Application>& application)
{
    IResourceManager::Initialize(application);

    // Resolve all resource loaders automatically
    GetApplication()->ResolveServices<IResourceLoader>(ResourceLoaders);
}

HResource SDL3ResourceManager::StoreResource(std::unique_ptr<IResource> resource, FName fullName)
{
    uint32_t resourceId = ++NextResourceId;
    ResourceInit::Init(*resource, resourceId, fullName);
    HResource handle = resource->GetHandle();
    Resources.emplace(resourceId, std::move(resource));
    return handle;
}

HResource SDL3ResourceManager::LoadResource(const std::string& path)
{
    FName fullName = MakeResourceFullName(path);
    if (auto existing = FindResourceRaw(fullName))
    {
        return existing->GetHandle();
    }

    std::vector<HResource> handles;
    LoadResources(path, handles);
    return handles.empty() ? HResource() : handles.front();
}

size_t SDL3ResourceManager::LoadResources(const std::string& path, std::vector<HResource>& outHandles)
{
    FName fullName = MakeResourceFullName(path);
    if (auto existing = FindResourceRaw(fullName))
    {
        outHandles.push_back(existing->GetHandle());
        return 1;
    }

    std::ifstream stream(path);
    if (!stream.is_open())
    {
        return 0;
    }

    auto ext = std::filesystem::path(path).extension().string();

    ResourceLoadArgs args = {
        .PlatformService = *Platform.get(),
        .FilePath = path,
        .Stream = stream
    };

    IResourceLoader* foundLoader = nullptr;
    for (auto&& loader : ResourceLoaders)
    {
        if (loader->CanLoad(args))
        {
            foundLoader = loader.get();
            break;
        }

        // Seek back to zero in case CanLoad read any of the stream
        stream.seekg(0);
    }

    if (!foundLoader)
    {
        return 0;
    }

    LogVerbose("Loading asset: {}", path);

    // Seek back to zero in case CanLoad read any of the stream
    stream.seekg(0);

    std::vector<std::unique_ptr<IResource>> loadedResources;
    if (!foundLoader->Load(args, loadedResources))
    {
        return 0;
    }



    LogVerbose("Loaded {} resources from asset {}", loadedResources.size(), path);

    for (auto&& resource : loadedResources)
    {
        auto handle = StoreResource(std::move(resource), fullName);
        outHandles.push_back(handle);
    }

    return loadedResources.size();
}

const IResource* SDL3ResourceManager::GetResourceRaw(HResource handle) const
{
    if (!handle.IsValid())
    {
        return nullptr;
    }

    auto iter = Resources.find(handle.Id);
    return iter != Resources.end() ? iter->second.get() : nullptr;
}

const IResource* SDL3ResourceManager::FindResourceRaw(FName name) const
{
    for (auto&& resource : Resources | std::views::values)
    {
        if (resource->GetFullName() == name)
        {
            return resource.get();
        }
    }
    return nullptr;
}

bool SDL3ResourceManager::ReleaseResource(HResource handle)
{
    if (!handle.IsValid())
    {
        return false;
    }

    auto iter = Resources.find(handle.Id);
    if (iter == Resources.end())
    {
        return false;
    }

    Resources.erase(iter);
    return true;
}

HResource SDL3ResourceManager::CreateRenderTarget(uint32_t w, uint32_t h)
{
    SDL_Texture* texture = SDL_CreateTexture(
        Platform->GetRenderer(),
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h);
    auto resource = std::make_unique<Texture2D>(texture, glm::u32vec2{ w, h });
    return StoreResource(std::move(resource));
}
