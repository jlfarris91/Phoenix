#include "SDL3ResourceManager.h"

namespace Phoenix::Renderer::SDL3
{
    SDL3ResourceManager::SDL3ResourceManager(std::shared_ptr<SDL3Context> context)
        : Context(std::move(context))
    {
    }

    SDL3ResourceManager::~SDL3ResourceManager()
    {
        for (auto& [id, entry] : Textures)
            SDL_DestroyTexture(entry.Texture);
    }

    HTexture SDL3ResourceManager::LoadTexture(const char* path)
    {
        SDL_Surface* surface = SDL_LoadBMP(path);
        if (!surface)
            return {};
        SDL_Texture* texture = SDL_CreateTextureFromSurface(Context->GetRenderer(), surface);
        SDL_DestroySurface(surface);
        if (!texture)
            return {};

        float w = 0.f, h = 0.f;
        SDL_GetTextureSize(texture, &w, &h);

        uint32_t id = NextTextureId++;
        Textures[id] = { texture, { w, h } };
        return { id };
    }

    void SDL3ResourceManager::ReleaseTexture(HTexture handle)
    {
        auto it = Textures.find(handle.Id);
        if (it == Textures.end())
            return;
        SDL_DestroyTexture(it->second.Texture);
        Textures.erase(it);
    }

    Vec2f SDL3ResourceManager::GetTextureSize(HTexture handle) const
    {
        auto it = Textures.find(handle.Id);
        return it != Textures.end() ? it->second.Size : Vec2f{};
    }

    HMesh2D SDL3ResourceManager::CreateMesh(const Vertex2D* vertices, uint32_t vertexCount,
                                             const uint16_t* indices,  uint32_t indexCount)
    {
        MeshEntry entry;
        entry.Vertices.assign(vertices, vertices + vertexCount);
        entry.Indices.resize(indexCount);
        for (uint32_t i = 0; i < indexCount; ++i)
            entry.Indices[i] = static_cast<int>(indices[i]);

        uint32_t id = NextMeshId++;
        Meshes[id] = std::move(entry);
        return { id };
    }

    HMesh2D SDL3ResourceManager::LoadMesh(const char* path)
    {
        // TODO: implement mesh file format loading
        return {};
    }

    void SDL3ResourceManager::ReleaseMesh(HMesh2D handle)
    {
        Meshes.erase(handle.Id);
    }

    HRenderTarget SDL3ResourceManager::CreateRenderTarget(int width, int height)
    {
        SDL_Texture* texture = SDL_CreateTexture(
            Context->GetRenderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            width, height);
        if (!texture)
            return {};

        uint32_t texId = NextTextureId++;
        Textures[texId] = { texture, { static_cast<float>(width), static_cast<float>(height) } };

        uint32_t rtId = NextRenderTargetId++;
        RenderTargets[rtId] = { { texId } };
        return { rtId };
    }

    void SDL3ResourceManager::ReleaseRenderTarget(HRenderTarget handle)
    {
        auto it = RenderTargets.find(handle.Id);
        if (it == RenderTargets.end())
            return;
        ReleaseTexture(it->second.TextureHandle);
        RenderTargets.erase(it);
    }

    HTexture SDL3ResourceManager::GetRenderTargetTexture(HRenderTarget handle) const
    {
        auto it = RenderTargets.find(handle.Id);
        return it != RenderTargets.end() ? it->second.TextureHandle : HTexture{};
    }

    SDL_Texture* SDL3ResourceManager::GetSDLTexture(HTexture handle) const
    {
        auto it = Textures.find(handle.Id);
        return it != Textures.end() ? it->second.Texture : nullptr;
    }

    const SDL3ResourceManager::MeshEntry* SDL3ResourceManager::GetMeshEntry(HMesh2D handle) const
    {
        auto it = Meshes.find(handle.Id);
        return it != Meshes.end() ? &it->second : nullptr;
    }
}
