#pragma once

#include "WorldViewWidget.h"
#include "../Controls/MemoryTool.h"

class MemoryView : public WorldViewWidget
{
public:

    void Initialize() override;

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;

private:

    MemoryTool MemoryTool;
};
