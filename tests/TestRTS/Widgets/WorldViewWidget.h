#pragma once

#include "Widget.h"
#include "PhoenixSim/SessionFwd.h"
#include "PhoenixSim/WorldsFwd.h"

class WorldViewWidget : public Widget
{
public:

    virtual void Render() override;

protected:

    virtual void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) = 0;
};
