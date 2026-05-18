
#include "SDLDebugState.h"

#include <SDL3/SDL_mouse.h>

using namespace Phoenix;

SDLDebugState::SDLDebugState(SDLViewport* viewport)
    : Viewport(viewport)
{
}

bool SDLDebugState::KeyDown(uint32 keycode) const
{
    auto iter = KeyStates.find(keycode);
    return iter != KeyStates.end() && iter->second;
}

bool SDLDebugState::KeyUp(uint32 keycode) const
{
    auto iter = KeyStates.find(keycode);
    return iter == KeyStates.end() || !iter->second;
}

bool SDLDebugState::KeyPressed(uint32 keycode) const
{
    if (!KeyDown(keycode))
        return false;

    auto prevIter = PrevKeyStates.find(keycode);
    return prevIter == PrevKeyStates.end() || !prevIter->second;
}

bool SDLDebugState::KeyReleased(uint32 keycode) const
{
    if (!KeyUp(keycode))
        return false;

    auto prevIter = PrevKeyStates.find(keycode);
    return prevIter != PrevKeyStates.end() && prevIter->second;
}

bool SDLDebugState::MouseButtonDown(uint8 button) const
{
    auto iter = MouseButtonStates.find(button);
    return iter != MouseButtonStates.end() && iter->second;
}

bool SDLDebugState::MouseButtonUp(uint8 button) const
{
    auto iter = MouseButtonStates.find(button);
    return iter == MouseButtonStates.end() || !iter->second;
}

bool SDLDebugState::MouseButtonPressed(uint8 button) const
{
    if (!MouseButtonDown(button))
        return false;

    auto prevIter = PrevMouseButtonStates.find(button);
    return prevIter == PrevMouseButtonStates.end() || !prevIter->second;
}

bool SDLDebugState::MouseButtonReleased(uint8 button) const
{
    if (!MouseButtonUp(button))
        return false;

    auto prevIter = PrevMouseButtonStates.find(button);
    return prevIter != PrevMouseButtonStates.end() && prevIter->second;
}

Vec2 SDLDebugState::GetWorldMousePos() const
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);

    return Viewport->ViewportPosToWorldPos(Viewport->WindowPosToViewportPos({ mx, my }));
}

void SDLDebugState::ProcessAppEvent(SDL_Event* event)
{
    PrevKeyStates = KeyStates;
    PrevMouseButtonStates = MouseButtonStates;

    if (event->type == SDL_EVENT_KEY_UP || event->type == SDL_EVENT_KEY_DOWN)
    {
        KeyStates[event->key.key] = event->type == SDL_EVENT_KEY_DOWN;

        SDL_Keymod mods = event->key.mod;
        KeyStates[SDL_KMOD_LCTRL] = mods & SDL_KMOD_LCTRL;
        KeyStates[SDL_KMOD_RCTRL] = mods & SDL_KMOD_RCTRL;
        KeyStates[SDL_KMOD_CTRL] = (mods & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL)) != 0;
        KeyStates[SDL_KMOD_LSHIFT] = mods & SDL_KMOD_LSHIFT;
        KeyStates[SDL_KMOD_RSHIFT] = mods & SDL_KMOD_RSHIFT;
        KeyStates[SDL_KMOD_SHIFT] = (mods & (SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT)) != 0;
        KeyStates[SDL_KMOD_LALT] = mods & SDL_KMOD_LALT;
        KeyStates[SDL_KMOD_RALT] = mods & SDL_KMOD_RALT;
        KeyStates[SDL_KMOD_ALT] = (mods & (SDL_KMOD_LALT | SDL_KMOD_RALT)) != 0;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        MouseButtonStates[event->button.button] = true;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        MouseButtonStates[event->button.button] = false;
    }
}
