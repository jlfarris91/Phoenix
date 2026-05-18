
#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>

#include <Phoenix.Sim.Debug/Debug.h>
#include <Phoenix.Sim/Platform.h>

#include "SDLViewport.h"

struct SDLDebugState : Phoenix::IDebugState
{
    SDLDebugState(SDLViewport* viewport);

    bool KeyDown(uint32_t keycode) const override;
    bool KeyUp(uint32_t keycode) const override;
    bool KeyPressed(uint32_t keycode) const override;
    bool KeyReleased(uint32_t keycode) const override;

    bool MouseButtonDown(uint8_t button) const override;
    bool MouseButtonUp(uint8_t button) const override;
    bool MouseButtonPressed(uint8_t button) const override;
    bool MouseButtonReleased(uint8_t button) const override;

    Phoenix::Vec2 GetWorldMousePos() const override;

    void ProcessAppEvent(SDL_Event* event);

    SDLViewport* Viewport;
    std::map<uint8_t, bool> MouseButtonStates;
    std::map<uint8_t, bool> PrevMouseButtonStates;
    std::map<SDL_Keycode, bool> KeyStates;
    std::map<SDL_Keycode, bool> PrevKeyStates;
};