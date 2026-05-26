#pragma once

#include <cstdint>

#include "RendererTypes.h"
#include "Phoenix/Services/IService.h"

namespace Phoenix::Renderer
{
    class IResourceManager : public IService
    {
        PHX_DECLARE_TYPE_DERIVED(IResourceManager, Phoenix::IService)

    public:
        virtual HTexture        LoadTexture(const char* path) = 0;
        virtual void            ReleaseTexture(HTexture handle) = 0;
        virtual glm::vec2       GetTextureSize(HTexture handle) const = 0;

        virtual HMesh2D         CreateMesh(const Vertex2D* vertices, uint32_t vertexCount, const uint16_t* indices,  uint32_t indexCount) = 0;
        virtual HMesh2D         LoadMesh(const char* path) = 0;
        virtual void            ReleaseMesh(HMesh2D handle) = 0;

        virtual HRenderTarget   CreateRenderTarget(int width, int height) = 0;
        virtual void            ReleaseRenderTarget(HRenderTarget handle) = 0;
        virtual HTexture        GetRenderTargetTexture(HRenderTarget handle) const = 0;
    };
}
