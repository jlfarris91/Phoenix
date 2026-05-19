#pragma once

#include <SDL3/SDL.h>
#include "Services/Service.h"

namespace Phoenix::Renderer::SDL3
{
    class SDL3Context : public Phoenix::IService
    {
        PHX_DECLARE_TYPE_DERIVED(SDL3Context, Phoenix::IService)
    public:
        SDL3Context(SDL_Window* window, SDL_Renderer* renderer);

        SDL_Window*   GetWindow()   const { return Window; }
        SDL_Renderer* GetRenderer() const { return Renderer; }

    private:
        SDL_Window*   Window   = nullptr;
        SDL_Renderer* Renderer = nullptr;
    };
}
