#pragma once

#include <imgui.h>

namespace Phoenix::ECS { class JobScheduler; }

// Renders a JobScheduler as a static wave-layout DAG using ImDrawList.
// One instance per scheduler view; stores scroll/zoom state independently.
class JobGraphPanel
{
public:
    // Draw the graph inside the current ImGui window.
    // Call between ImGui::Begin / ImGui::End.
    void Draw(const Phoenix::ECS::JobScheduler& scheduler);

private:
    ImVec2 ScrollOffset = {0.f, 0.f};
    bool   Dragging     = false;
    ImVec2 DragStart    = {0.f, 0.f};
};
