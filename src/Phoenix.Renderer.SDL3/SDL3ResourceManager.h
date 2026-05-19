#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include "IResourceManager.h"
#include "SDL3Context.h"

namespace Phoenix::Renderer::SDL3
{
    class SDL3ResourceManager : public Phoenix::Renderer::IResourceManager
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3ResourceManager, Phoenix::Renderer::IResourceManager)
    public:
        explicit SDL3ResourceManager(std::shared_ptr<SDL3Context> context);
        ~SDL3ResourceManager() override;

        HTexture LoadTexture(const char* path) override;
        void     ReleaseTexture(HTexture handle) override;
        Vec2f    GetTextureSize(HTexture handle) const override;

        HMesh2D CreateMesh(const Vertex2D* vertices, uint32_t vertexCount,
                           const uint16_t* indices,  uint32_t indexCount) override;
        HMesh2D LoadMesh(const char* path) override;
        void    ReleaseMesh(HMesh2D handle) override;

        HRenderTarget CreateRenderTarget(int width, int height) override;
        void          ReleaseRenderTarget(HRenderTarget handle) override;
        HTexture      GetRenderTargetTexture(HRenderTarget handle) const override;

        SDL_Texture* GetSDLTexture(HTexture handle) const;

        struct MeshEntry
        {
            std::vector<Vertex2D> Vertices;
            std::vector<int>      Indices;
        };

        const MeshEntry* GetMeshEntry(HMesh2D handle) const;

    private:
        struct TextureEntry
        {
            SDL_Texture* Texture = nullptr;
            Vec2f        Size;
        };

        struct RenderTargetEntry
        {
            HTexture TextureHandle;  // registered in Textures; lifetime tied to this entry
        };

        std::shared_ptr<SDL3Context>                    Context;
        std::unordered_map<uint32_t, TextureEntry>      Textures;
        std::unordered_map<uint32_t, MeshEntry>         Meshes;
        std::unordered_map<uint32_t, RenderTargetEntry> RenderTargets;
        uint32_t                                        NextTextureId      = 1;
        uint32_t                                        NextMeshId         = 1;
        uint32_t                                        NextRenderTargetId = 1;
    };
}
