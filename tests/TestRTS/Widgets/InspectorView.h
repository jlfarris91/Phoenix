#pragma once

#include "WorldViewWidget.h"

class InspectorView : public WorldViewWidget
{
public:

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;
};