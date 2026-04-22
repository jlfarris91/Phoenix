#include "JobGraphPanel.h"

#include <algorithm>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_internal.h>

#include "PhoenixSim/ECS/JobScheduler.h"
#include "PhoenixSim/ECS/SystemJob.h"
#include "PhoenixSim/Name.h"

using namespace Phoenix;
using namespace Phoenix::ECS;

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------

static constexpr float NodeW       = 160.f;
static constexpr float NodeH       = 40.f;
static constexpr float WaveSpacingX = 220.f;
static constexpr float NodeSpacingY = 60.f;
static constexpr float CanvasPadX  = 24.f;
static constexpr float CanvasPadY  = 24.f;
static constexpr float Rounding    = 6.f;
static constexpr float ArrowSize   = 7.f;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string NameToString(FName name)
{
    if (FName::IsNoneOrEmpty(name))
        return "(unnamed)";
    const char* str = FName::GetNameEntry(name);
    if (str && str[0] != '\0')
    {
        // Strip namespace qualifiers — show only the last segment after ::
        std::string full(str);
        auto pos = full.rfind("::");
        return pos != std::string::npos ? full.substr(pos + 2) : full;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%08X", (hash32_t)name);
    return buf;
}

// Assign each job a wave index (0 = no predecessors).
static std::vector<int> ComputeWaves(const JobScheduler& sched)
{
    uint32 n = sched.GetJobCount();
    std::vector<int> wave(n, 0);
    for (uint32 i = 0; i < n; ++i)
        for (uint32 succ : sched.GetJobSuccessors(i))
            wave[succ] = std::max(wave[succ], wave[i] + 1);
    return wave;
}

// Draw an arrow from p0 to p1 on dl.
static void DrawArrow(ImDrawList* dl, ImVec2 p0, ImVec2 p1, ImU32 col)
{
    dl->AddLine(p0, p1, col, 1.5f);

    // Arrowhead
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.f) return;
    dx /= len; dy /= len;

    ImVec2 left  = { p1.x - ArrowSize * dx + ArrowSize * 0.5f * dy,
                     p1.y - ArrowSize * dy - ArrowSize * 0.5f * dx };
    ImVec2 right = { p1.x - ArrowSize * dx - ArrowSize * 0.5f * dy,
                     p1.y - ArrowSize * dy + ArrowSize * 0.5f * dx };
    dl->AddTriangleFilled(p1, left, right, col);
}

// ---------------------------------------------------------------------------
// JobGraphPanel::Draw
// ---------------------------------------------------------------------------

void JobGraphPanel::Draw(const JobScheduler& sched)
{
    uint32 jobCount = sched.GetJobCount();
    if (jobCount == 0)
    {
        ImGui::TextDisabled("(no jobs registered)");
        return;
    }

    // ------------------------------------------------------------------
    // Compute layout
    // ------------------------------------------------------------------
    std::vector<int> wave = ComputeWaves(sched);

    int waveCount = *std::max_element(wave.begin(), wave.end()) + 1;

    // Count how many nodes per wave so we can position them vertically.
    std::vector<int> waveSize(waveCount, 0);
    for (uint32 i = 0; i < jobCount; ++i)
        waveSize[wave[i]]++;

    // Assign a per-wave slot index to each job.
    std::vector<int> slot(jobCount);
    std::vector<int> cursor(waveCount, 0);
    for (uint32 i = 0; i < jobCount; ++i)
        slot[i] = cursor[wave[i]]++;

    // Node centres in canvas-local space.
    std::vector<ImVec2> centre(jobCount);
    for (uint32 i = 0; i < jobCount; ++i)
    {
        int w = wave[i];
        int s = slot[i];
        int total = waveSize[w];

        centre[i] = {
            CanvasPadX + w * WaveSpacingX + NodeW * 0.5f,
            CanvasPadY + s * NodeSpacingY + NodeH * 0.5f
        };
        (void)total;
    }

    // Canvas size needed to fit all nodes.
    float canvasW = CanvasPadX * 2 + waveCount * WaveSpacingX;
    float canvasH = CanvasPadY * 2 + (*std::max_element(waveSize.begin(), waveSize.end())) * NodeSpacingY;

    // ------------------------------------------------------------------
    // Scrollable child region
    // ------------------------------------------------------------------
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::SetNextWindowContentSize({ canvasW, canvasH });
    ImGui::BeginChild("##jgraph_scroll", avail, false,
                      ImGuiWindowFlags_HorizontalScrollbar |
                      ImGuiWindowFlags_NoMove);

    ImDrawList* dl     = ImGui::GetWindowDrawList();
    ImVec2      origin = ImGui::GetCursorScreenPos();

    // ------------------------------------------------------------------
    // Draw edges first (behind nodes)
    // ------------------------------------------------------------------
    ImU32 edgeCol = IM_COL32(160, 160, 160, 200);

    for (uint32 i = 0; i < jobCount; ++i)
    {
        ImVec2 from = { origin.x + centre[i].x + NodeW * 0.5f,
                        origin.y + centre[i].y };

        for (uint32 succ : sched.GetJobSuccessors(i))
        {
            ImVec2 to = { origin.x + centre[succ].x - NodeW * 0.5f,
                          origin.y + centre[succ].y };
            DrawArrow(dl, from, to, edgeCol);
        }
    }

    // ------------------------------------------------------------------
    // Draw nodes
    // ------------------------------------------------------------------
    ImU32 nodeBg     = IM_COL32( 50,  55,  65, 240);
    ImU32 nodeBorder = IM_COL32(100, 110, 130, 255);
    ImU32 textCol    = IM_COL32(230, 230, 230, 255);
    ImU32 readCol    = IM_COL32( 80, 180,  80, 220);
    ImU32 writeCol   = IM_COL32(220, 140,  40, 220);
    ImU32 badgeFg    = IM_COL32(  0,   0,   0, 255);

    for (uint32 i = 0; i < jobCount; ++i)
    {
        ImVec2 tl = { origin.x + centre[i].x - NodeW * 0.5f,
                      origin.y + centre[i].y - NodeH * 0.5f };
        ImVec2 br = { tl.x + NodeW, tl.y + NodeH };

        dl->AddRectFilled(tl, br, nodeBg, Rounding);
        dl->AddRect(tl, br, nodeBorder, Rounding, 0, 1.5f);

        // Job name centred in upper half of node
        std::string name = sched.GetJobName(i);
        ImVec2 textSz = ImGui::CalcTextSize(name.c_str());
        ImVec2 textPos = { tl.x + (NodeW - textSz.x) * 0.5f,
                           tl.y + (NodeH * 0.5f - textSz.y) * 0.5f };
        dl->AddText(textPos, textCol, name.c_str());

        // Access badges along the bottom strip
        const SystemAccessDescriptor& access = sched.GetJobAccess(i);
        float badgeX = tl.x + 4.f;
        float badgeY = tl.y + NodeH * 0.5f + 2.f;
        float badgeH = NodeH * 0.5f - 4.f;

        auto DrawBadge = [&](const char* label, ImU32 bg)
        {
            ImVec2 sz = ImGui::CalcTextSize(label);
            float bw = sz.x + 6.f;
            if (badgeX + bw > br.x - 2.f) return; // clip
            ImVec2 btl = { badgeX, badgeY };
            ImVec2 bbr = { badgeX + bw, badgeY + badgeH };
            dl->AddRectFilled(btl, bbr, bg, 3.f);
            dl->AddText({ btl.x + 3.f, btl.y + (badgeH - sz.y) * 0.5f }, badgeFg, label);
            badgeX += bw + 3.f;
        };

        for (const auto& c : access.Components)
        {
            std::string label = NameToString(c.ComponentId);
            if (label.size() > 8) label = label.substr(0, 7) + "…";
            ImU32 bg = (c.Access == EComponentAccess::Read) ? readCol : writeCol;
            DrawBadge(label.c_str(), bg);
        }
        for (const auto& f : access.Features)
        {
            std::string label = NameToString(f.FeatureId);
            if (label.size() > 8) label = label.substr(0, 7) + "…";
            ImU32 bg = (f.Access == EComponentAccess::Read) ? readCol : writeCol;
            DrawBadge(label.c_str(), bg);
        }

        // Invisible button for tooltip
        ImGui::SetCursorScreenPos(tl);
        ImGui::InvisibleButton(("##jgn" + std::to_string(i)).c_str(), { NodeW, NodeH });
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(name.c_str());
            ImGui::Separator();
            uint32 preds = sched.GetJobPredecessorCount(i);
            uint32 succs = (uint32)sched.GetJobSuccessors(i).size();
            ImGui::Text("Predecessors: %u  Successors: %u", preds, succs);
            if (!access.Components.empty())
            {
                ImGui::Separator();
                ImGui::TextDisabled("Components");
                for (const auto& c : access.Components)
                    ImGui::Text("  %s %s", NameToString(c.ComponentId).c_str(),
                                c.Access == EComponentAccess::Read ? "(R)" : "(W)");
            }
            if (!access.Features.empty())
            {
                ImGui::Separator();
                ImGui::TextDisabled("Features");
                for (const auto& f : access.Features)
                    ImGui::Text("  %s %s", NameToString(f.FeatureId).c_str(),
                                f.Access == EComponentAccess::Read ? "(R)" : "(W)");
            }
            ImGui::EndTooltip();
        }
    }

    // Reserve canvas space so the scroll region knows its extent.
    ImGui::SetCursorScreenPos({ origin.x + canvasW, origin.y + canvasH });
    ImGui::Dummy({ 0, 0 });

    ImGui::EndChild();
}
