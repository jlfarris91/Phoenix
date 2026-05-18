#pragma once

#include <Phoenix.Sim/WorldsFwd.h>
#include <Phoenix/Reflection/Registration.h>

namespace Phoenix
{
    class IDebugState;
    class IDebugRenderer;
}

class ITool
{
    PHX_DECLARE_TYPE_INTERFACE(ITool)

public:

    virtual ~ITool() = default;

    virtual const char* GetDescription() const { return ""; }

    virtual void OnActivated() {}
    virtual void OnDeactivated() {}
    virtual void OnAppRenderWorld(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, Phoenix::IDebugRenderer& renderer) {}
    virtual void OnAppRenderUI() {}
    virtual void OnAppEvent(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, const void* eventData) {}
};