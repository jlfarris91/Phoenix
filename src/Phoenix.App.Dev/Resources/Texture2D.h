#pragma once

#include <glm/vec2.hpp>
#include <SDL3/SDL_render.h>

#include "Resource.h"
#include "ResourceLoader.h"

namespace Phoenix::App::Dev
{
    class Texture2D : public Renderer::Resource
    {
        PHX_DECLARE_TYPE_DERIVED(Texture2D, Renderer::Resource);
    public:
        Texture2D(SDL_Texture* texture, glm::vec2 size);
        ~Texture2D() override;

        FName GetResourceType() const override
        {
            return StaticTypeName<Texture2D>::TypeId;
        }

        SDL_Texture* GetSDLTexture() const;

        glm::vec2 GetSize() const;
        float GetWidth() const;
        float GetHeight() const;

        void ReleaseResources() override;

    private:
        SDL_Texture* Texture = nullptr;
        glm::vec2 Size = glm::vec2(0.0f);
    };

    class Texture2DLoader : public Renderer::IResourceLoader
    {
        PHX_DECLARE_TYPE_DERIVED(Texture2DLoader, Renderer::IResourceLoader);
    public:
        bool CanLoad(const Renderer::ResourceLoadArgs& args) const override;
        size_t Load(const Renderer::ResourceLoadArgs& args, std::vector<std::unique_ptr<Renderer::IResource>>& outResources) override;
    };
}
