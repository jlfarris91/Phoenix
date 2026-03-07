
#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>

#include <PhoenixSim/Debug/Debug.h>
#include <PhoenixSim/Platform.h>

#include "SDLViewport.h"

namespace Phoenix
{
    struct SDLDebugState : IDebugState
    {
        SDLDebugState(SDLViewport* viewport);

        bool KeyDown(uint32 keycode) const override;
        bool KeyUp(uint32 keycode) const override;
        bool KeyPressed(uint32 keycode) const override;
        bool KeyReleased(uint32 keycode) const override;

        bool MouseButtonDown(uint8 button) const override;
        bool MouseButtonUp(uint8 button) const override;
        bool MouseButtonPressed(uint8 button) const override;
        bool MouseButtonReleased(uint8 button) const override;

        Vec2 GetWorldMousePos() const override;

        void ProcessAppEvent(SDL_Event* event);

        SDLViewport* Viewport;
        std::map<uint8, bool> MouseButtonStates;
        std::map<uint8, bool> PrevMouseButtonStates;
        std::map<SDL_Keycode, bool> KeyStates;
        std::map<SDL_Keycode, bool> PrevKeyStates;
    };
}
