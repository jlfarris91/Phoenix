#pragma once

#include "WorldViewWidget.h"

class BlackboardView : public WorldViewWidget
{
protected:
    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;
};
