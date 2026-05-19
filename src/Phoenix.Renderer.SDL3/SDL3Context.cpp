#include "SDL3Context.h"

namespace Phoenix::Renderer::SDL3
{
    SDL3Context::SDL3Context(SDL_Window* window, SDL_Renderer* renderer)
        : Window(window), Renderer(renderer)
    {
    }
}
