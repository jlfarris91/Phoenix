#pragma once

#include <filesystem>
#include <glm/glm.hpp>

#include "Resource.h"
#include "ResourceLoader.h"
#include "Phoenix/Name.h"

namespace Phoenix::App::Dev
{
    struct LineMesh2DVertex
    {
        glm::vec2 Position;
        glm::vec4 Color;
    };

    struct LineMesh2DSocket
    {
        FName Id;
        glm::mat4 Transform;
    };

    class LineMesh2D : public Renderer::Resource
    {
        PHX_DECLARE_TYPE_DERIVED(LineMesh2D, Renderer::Resource);
    public:

        FName GetResourceType() const override;

        std::vector<LineMesh2DVertex> Vertices;
        std::vector<uint32_t> Indices;
        std::unordered_map<FName, LineMesh2DSocket> Sockets;
    };

    class LineMesh2DLoader : public Renderer::IResourceLoader
    {
        PHX_DECLARE_TYPE_DERIVED(LineMesh2DLoader, Renderer::IResourceLoader);
    public:
        bool CanLoad(const Renderer::ResourceLoadArgs& args) const override;
        size_t Load(const Renderer::ResourceLoadArgs& args, std::vector<std::unique_ptr<Renderer::IResource>>& outResources) override;
    };
}
