#pragma once

#include "../Controls/JobGraphPanel.h"
#include "WorldViewWidget.h"

class JobGraphPanel;

class JobGraphView : public WorldViewWidget
{
public:

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;

private:

    JobGraphPanel JobGraphPreUpdate;
    JobGraphPanel JobGraphUpdate;
    JobGraphPanel JobGraphPostUpdate;
    std::unordered_map<uint32_t, JobGraphPanel> NamedJobGraphPanels;
};
