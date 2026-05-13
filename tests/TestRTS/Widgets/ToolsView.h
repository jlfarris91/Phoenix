#pragma once

#include "WorldViewWidget.h"

class ToolManager;

class ToolsView : public WorldViewWidget
{
public:

    ToolsView(ToolManager* toolManager);

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;
    
private:

    ToolManager* ToolManager;
};
