#pragma once

#include <memory>
#include <set>
#include <vector>

class ITool;

class ToolManager
{
public:

    ITool* RegisterTool(std::unique_ptr<ITool>&& tool);

    std::vector<ITool*> GetTools() const;
    std::vector<ITool*> GetActiveTools() const;

    bool ActivateTool(ITool* tool);
    bool DeactivateTool(ITool* tool);

    bool IsToolActive(const ITool* tool) const;

private:

    std::vector<std::unique_ptr<ITool>> Tools;
    std::set<ITool*> ActiveTools;
};
