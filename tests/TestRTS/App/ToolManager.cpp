#include "ToolManager.h"

#include "Tool.h"

ITool* ToolManager::RegisterTool(std::unique_ptr<ITool>&& tool)
{
    Tools.push_back(std::move(tool));
    return Tools.back().get();
}

std::vector<ITool*> ToolManager::GetTools() const
{
    std::vector<ITool*> tools;
    tools.reserve(Tools.size());
    for (const auto& tool : Tools)
    {
        tools.push_back(tool.get());
    }
    return tools;
}

std::vector<ITool*> ToolManager::GetActiveTools() const
{
    std::vector<ITool*> activeTools;
    activeTools.reserve(ActiveTools.size());
    for (const auto& tool : ActiveTools)
    {
        activeTools.push_back(tool);
    }
    return activeTools;
}

bool ToolManager::ActivateTool(ITool* tool)
{
    if (IsToolActive(tool))
    {
        return false;
    }
    ActiveTools.insert(tool);
    return true;
}

bool ToolManager::DeactivateTool(ITool* tool)
{
    if (!IsToolActive(tool))
    {
        return false;
    }
    ActiveTools.erase(tool);
    return true;
}

bool ToolManager::IsToolActive(const ITool* tool) const
{
    return ActiveTools.contains(const_cast<ITool*>(tool));
}
