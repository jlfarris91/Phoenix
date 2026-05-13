#pragma once

#include "WorldViewWidget.h"

class DebugView : public WorldViewWidget
{
public:

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;
};