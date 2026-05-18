#include "FlameGraph.h"

#include <algorithm>
#include <cmath>

static constexpr float  kRowGap      = 2.0f;
static constexpr float  kMinSegW     = 1.0f;
static constexpr float  kDragThresh  = 4.0f;   // pixels before a right-drag becomes a pan
static constexpr float  kZoomStep    = 0.85f;  // zoom factor per scroll tick
static constexpr double kMinViewSpan = 1.0;    // minimum viewable domain width

static void ClampView(FlameGraphView& view, double span, uint64_t totalUnits)
{
    const double domain = static_cast<double>(totalUnits);
    if (view.ViewStart < 0.0) { view.ViewStart = 0.0; view.ViewEnd = span; }
    if (view.ViewEnd > domain) { view.ViewEnd = domain; view.ViewStart = domain - span; }
    if (view.ViewStart < 0.0)   view.ViewStart = 0.0;
}

void DrawFlameGraph(
    const char*                           id,
    FlameGraphView&                       view,
    const std::vector<FlameGraphSegment>& segments,
    uint64_t                              totalUnits,
    float                                 rowHeight,
    FlameGraphOverlayFn                   overlayFn,
    FlameGraphClickFn                     clickFn)
{
    if (totalUnits == 0 || segments.empty()) return;

    if (view.ViewEnd <= view.ViewStart)
    {
        view.ViewStart = 0.0;
        view.ViewEnd   = static_cast<double>(totalUnits);
    }

    int maxDepth = 0;
    for (const auto& seg : segments)
        maxDepth = std::max(maxDepth, seg.Depth);

    const float  totalH = static_cast<float>(maxDepth + 1) * (rowHeight + kRowGap) - kRowGap;
    const float  avail  = ImGui::GetContentRegionAvail().x;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 size   = {avail, totalH};

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(origin, {origin.x + size.x, origin.y + size.y}, IM_COL32(20, 20, 20, 255));

    // Canvas invisible button — captures left-click and hover; right-drag is
    // tracked via global mouse state so it doesn't need a separate button.
    ImGui::SetCursorScreenPos(origin);
    ImGui::InvisibleButton(id, size);
    const bool isActive      = ImGui::IsItemActive();
    const bool isHovered     = ImGui::IsItemHovered();
    const bool isDeactivated = ImGui::IsItemDeactivated();

    // ── Coordinate helpers ────────────────────────────────────────────────────

    const double viewSpan  = view.ViewEnd - view.ViewStart;
    const float  pxPerUnit = viewSpan > 0.0
        ? static_cast<float>(static_cast<double>(size.x) / viewSpan) : 0.0f;

    auto UnitToPx = [&](double unit) -> float {
        return origin.x + static_cast<float>((unit - view.ViewStart) * pxPerUnit);
    };

    // ── Zoom (scroll wheel) ───────────────────────────────────────────────────

    if (isHovered && pxPerUnit > 0.0f)
    {
        const float scrollY = ImGui::GetIO().MouseWheel;
        if (scrollY != 0.0f)
        {
            const double cursorUnit = view.ViewStart +
                static_cast<double>(ImGui::GetMousePos().x - origin.x) / pxPerUnit;
            const double factor  = std::pow(static_cast<double>(kZoomStep),
                                            static_cast<double>(scrollY));
            double newSpan = std::max(kMinViewSpan,
                             std::min(viewSpan * factor, static_cast<double>(totalUnits)));
            const double frac = viewSpan > 0.0 ? (cursorUnit - view.ViewStart) / viewSpan : 0.5;
            view.ViewStart = cursorUnit - frac * newSpan;
            view.ViewEnd   = view.ViewStart + newSpan;
            ClampView(view, newSpan, totalUnits);
        }
    }

    // ── Right-drag pan ────────────────────────────────────────────────────────

    {
        const bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
        const bool rightDrag = ImGui::IsMouseDragging(ImGuiMouseButton_Right, kDragThresh);

        if (!view._RightDragging && isHovered && rightDrag)
        {
            view._RightDragging      = true;
            view._DragStartX         = ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Right].x;
            view._DragStartViewStart = view.ViewStart;
        }

        if (view._RightDragging)
        {
            if (!rightDown)
            {
                view._RightDragging = false;
            }
            else if (pxPerUnit > 0.0f)
            {
                const double delta = -static_cast<double>(ImGui::GetMousePos().x - view._DragStartX)
                                     / pxPerUnit;
                view.ViewStart = view._DragStartViewStart + delta;
                view.ViewEnd   = view.ViewStart + viewSpan;
                ClampView(view, viewSpan, totalUnits);
            }
        }
    }

    // ── Double-click detection (fires on 2nd click-DOWN) ──────────────────────

    if (isHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        view._WasDoubleClick = true;

    // ── Draw segments + find click target ─────────────────────────────────────

    const ImVec2 mousePos = ImGui::GetMousePos();
    const bool   checkClick = isDeactivated && !view._RightDragging;

    const FlameGraphSegment* clickedSeg = nullptr;

    dl->PushClipRect(origin, {origin.x + size.x, origin.y + size.y}, true);

    for (const auto& seg : segments)
    {
        if (seg.Size == 0) continue;

        const double segEnd = static_cast<double>(seg.Start + seg.Size);
        if (segEnd <= view.ViewStart || static_cast<double>(seg.Start) >= view.ViewEnd)
            continue;

        const float ux0 = UnitToPx(static_cast<double>(seg.Start));
        const float ux1 = UnitToPx(segEnd);
        const float px0 = std::max(ux0, origin.x);
        const float px1 = std::min(ux1, origin.x + size.x);
        if (px1 - px0 < kMinSegW) continue;

        const float py0 = origin.y + static_cast<float>(seg.Depth) * (rowHeight + kRowGap);
        const float py1 = py0 + rowHeight;

        dl->AddRectFilled({px0, py0}, {px1 - 1.0f, py1}, seg.Color);
        dl->AddLine({px1 - 1.0f, py0}, {px1 - 1.0f, py1}, IM_COL32(0, 0, 0, 120));

        if (seg.Label && px1 - px0 > 24.0f)
        {
            dl->PushClipRect({px0 + 2.0f, py0}, {px1 - 2.0f, py1}, true);
            const float ty = py0 + (rowHeight - ImGui::GetTextLineHeight()) * 0.5f;
            dl->AddText({px0 + 3.0f, ty}, IM_COL32(255, 255, 255, 220), seg.Label);
            dl->PopClipRect();
        }

        if (overlayFn)
            overlayFn(seg, dl, {ux0, py0}, {ux1, py1});

        if (checkClick &&
            mousePos.x >= px0 && mousePos.x < px1 &&
            mousePos.y >= py0 && mousePos.y < py1)
        {
            clickedSeg = &seg;
        }
    }

    dl->PopClipRect();

    // ── Click / double-click actions ──────────────────────────────────────────

    if (isDeactivated)
    {
        if (clickedSeg)
        {
            if (view._WasDoubleClick)
            {
                // Double-click: re-frame to the segment bounds.
                view.ViewStart = static_cast<double>(clickedSeg->Start);
                view.ViewEnd   = static_cast<double>(clickedSeg->Start + clickedSeg->Size);
            }
            if (clickFn) clickFn(*clickedSeg);
        }
        view._WasDoubleClick = false;
    }

    // ── Horizontal scrollbar ──────────────────────────────────────────────────

    const double totalD      = static_cast<double>(totalUnits);
    const bool   showScroll  = viewSpan < totalD * 0.995;

    // Cursor is now just below the canvas (InvisibleButton advanced it).
    // Position explicitly for determinism.
    ImGui::SetCursorScreenPos({origin.x, origin.y + size.y});

    if (showScroll)
    {
        constexpr float kScrollH = 6.0f;

        const ImVec2 trackPos = {origin.x, origin.y + size.y};

        dl->AddRectFilled(trackPos, {trackPos.x + avail, trackPos.y + kScrollH},
                          IM_COL32(30, 30, 30, 220));

        ImGui::InvisibleButton("##fgscroll", {avail, kScrollH});
        const bool scrollHov = ImGui::IsItemHovered();
        const bool scrollAct = ImGui::IsItemActive();

        if (scrollAct)
        {
            const float mx        = std::max(0.0f, std::min(ImGui::GetMousePos().x - trackPos.x, avail));
            const double newCenter = (mx / avail) * totalD;
            view.ViewStart = std::max(0.0, std::min(newCenter - viewSpan * 0.5, totalD - viewSpan));
            view.ViewEnd   = view.ViewStart + viewSpan;
        }

        const float tX0      = trackPos.x + avail * static_cast<float>(view.ViewStart / totalD);
        const float tW       = std::max(8.0f, avail * static_cast<float>(viewSpan / totalD));
        const ImU32 thumbCol = (scrollHov || scrollAct)
            ? IM_COL32(160, 160, 160, 230)
            : IM_COL32(90,  90,  90,  200);
        dl->AddRectFilled({tX0, trackPos.y}, {tX0 + tW, trackPos.y + kScrollH}, thumbCol);
    }
}
