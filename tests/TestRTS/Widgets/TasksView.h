#pragma once

#include "WorldViewWidget.h"

class TasksView : public WorldViewWidget
{
protected:
    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;
};