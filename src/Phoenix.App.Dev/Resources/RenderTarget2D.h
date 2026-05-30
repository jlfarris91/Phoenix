#pragma once

#include "Texture2D.h"

namespace Phoenix::App::Dev
{
    class RenderTarget2D : public Texture2D
    {
        PHX_DECLARE_TYPE_DERIVED(RenderTarget2D, Texture2D);
    public:
        RenderTarget2D(SDL_Texture* texture, glm::vec2 size);

        FName GetResourceType() const override
        {
            return StaticTypeName<RenderTarget2D>::TypeId;
        }
    };
}
