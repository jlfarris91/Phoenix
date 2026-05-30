#pragma once

#include <glm/vec2.hpp>
#include <SDL3/SDL_render.h>

#include "Resource.h"
#include "ResourceLoader.h"

namespace Phoenix::App::Dev
{
    class Texture2D : public Renderer::IResource
    {
        PHX_DECLARE_TYPE_DERIVED(Texture2D, Renderer::IResource);
    public:
        Texture2D(SDL_Texture* texture, glm::u32vec2 size);
        ~Texture2D() override;

        FName GetResourceType() const override
        {
            return StaticTypeName<Texture2D>::TypeId;
        }

        SDL_Texture* GetSDLTexture() const;

        glm::u32vec2 GetSize() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        void ReleaseResources() override;

    private:
        SDL_Texture* Texture = nullptr;
        glm::u32vec2 Size = glm::u32vec2(0);
    };

    class Texture2DLoader : public Renderer::IResourceLoader
    {
        PHX_DECLARE_TYPE_DERIVED(Texture2DLoader, Renderer::IResourceLoader);
    public:
        bool CanLoad(const Renderer::ResourceLoadArgs& args) const override;
        size_t Load(const Renderer::ResourceLoadArgs& args, std::vector<std::unique_ptr<Renderer::IResource>>& outResources) override;
    };
}
