#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <imgui.h>

// One colored segment in the flame graph.
// Start + Size are in "unit space" (typically bytes).
// Depth is the row index, 0 = top row.
// Tag is caller-defined metadata passed verbatim to callbacks.
struct FlameGraphSegment
{
    uint64_t    Start  = 0;
    uint64_t    Size   = 0;
    int         Depth  = 0;
    ImU32       Color  = IM_COL32(100, 100, 100, 255);
    const char* Label  = nullptr;
    uintptr_t   Tag    = 0;
};

// Persistent zoom/pan state — one per flame graph instance.
struct FlameGraphView
{
    double ViewStart = 0.0;
    double ViewEnd   = 0.0;  // ViewEnd == ViewStart means "uninitialized → show full range"

    // Internal input state — do not modify.
    bool   _RightDragging      = false;
    float  _DragStartX         = 0.0f;
    double _DragStartViewStart = 0.0;
    bool   _WasDoubleClick     = false;
};

// Called after each visible segment is drawn so callers can paint overlays
// (e.g., dirty-page highlights). pixelMin/pixelMax are the full unclipped
// pixel extents of the segment; dl is the active draw list.
using FlameGraphOverlayFn = std::function<void(
    const FlameGraphSegment& seg,
    ImDrawList*              dl,
    ImVec2                   pixelMin,
    ImVec2                   pixelMax)>;

// Called when the user clicks a segment.
using FlameGraphClickFn = std::function<void(const FlameGraphSegment& seg)>;

// Draw a generic, zoomable flame graph.
//
//   id         — ImGui widget ID string (must be unique in current window)
//   view       — zoom/pan state (modified in place)
//   segments   — flat list of all segments at all depths, any order
//   totalUnits — total domain width (the full extent of unit space)
//   rowHeight  — pixel height per depth row
//   overlayFn  — optional; called after each visible segment is drawn
//   clickFn    — optional; called when a segment is clicked
//
// Interactions:
//   Mouse wheel      — zoom around cursor
//   Right-drag       — pan
//   Left-click       — invoke clickFn (select)
//   Left-dbl-click   — re-frame view to clicked segment + invoke clickFn
void DrawFlameGraph(
    const char*                           id,
    FlameGraphView&                       view,
    const std::vector<FlameGraphSegment>& segments,
    uint64_t                              totalUnits,
    float                                 rowHeight = 20.0f,
    FlameGraphOverlayFn                   overlayFn = nullptr,
    FlameGraphClickFn                     clickFn   = nullptr);
