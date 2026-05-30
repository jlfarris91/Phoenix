#include "Texture2D.h"

#include <filesystem>
#include <string>

#include "SDL3PlatformService.h"
#include "Phoenix/Logging.h"

Phoenix::App::Dev::Texture2D::Texture2D(SDL_Texture* texture, glm::vec2 size)
    : Texture(texture)
    , Size(size)
{
}

Phoenix::App::Dev::Texture2D::~Texture2D()
{
    if (Texture)
    {
        SDL_DestroyTexture(Texture);
        Texture = nullptr;
    }
}

SDL_Texture* Phoenix::App::Dev::Texture2D::GetSDLTexture() const
{
    return Texture;
}

glm::vec2 Phoenix::App::Dev::Texture2D::GetSize() const
{
    return Size;
}

float Phoenix::App::Dev::Texture2D::GetWidth() const
{
    return Size.x;
}

float Phoenix::App::Dev::Texture2D::GetHeight() const
{
    return Size.y;
}

void Phoenix::App::Dev::Texture2D::ReleaseResources()
{
    if (Texture)
    {
        SDL_DestroyTexture(Texture);
        Texture = nullptr;
    }
}

bool Phoenix::App::Dev::Texture2DLoader::CanLoad(const Renderer::ResourceLoadArgs& args) const
{
    auto ext = std::filesystem::path(args.FilePath).extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char c)
    {
        return std::tolower(c);
    });
    return ext == "bmp" || ext == "png";
}

size_t Phoenix::App::Dev::Texture2DLoader::Load(
    const Renderer::ResourceLoadArgs& args,
    std::vector<std::unique_ptr<Renderer::IResource>>& outResources)
{
    SDL_Surface* surface = SDL_LoadSurface(args.FilePath.c_str());
    if (!surface)
    {
        LogError("Failed to load texture from '{}': {}", args.FilePath, SDL_GetError());
        return 0;
    }

    auto platform = static_cast<SDL3PlatformService&>(args.PlatformService);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(platform.GetRenderer(), surface);
    SDL_DestroySurface(surface);
    if (!texture)
    {
        return 0;
    }

    float w = 0.f, h = 0.f;
    SDL_GetTextureSize(texture, &w, &h);

    auto resource = std::make_unique<Texture2D>(texture, glm::vec2{ w, h });
    outResources.push_back(std::move(resource));
    return 1;
}