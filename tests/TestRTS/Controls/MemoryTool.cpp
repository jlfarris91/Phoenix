
#include "MemoryTool.h"

#include <algorithm>
#include <cstdio>

#include <imgui.h>

#include "FlameGraph.h"

#include <PhoenixSim/Reflection/TypeDescriptor.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/Worlds.h>

using namespace Phoenix;

// ── constants & color helpers ─────────────────────────────────────────────────

static constexpr float kFadeDuration  = 0.5f;
static constexpr float kBarH          = 21.0f;
static constexpr float kDetailBarH    = 20.0f;

// Rotating palette used to color struct fields in the detail view.
static const ImU32 kFieldColors[] = {
    IM_COL32(220,  80,  80, 255),  // red
    IM_COL32( 80, 200,  80, 255),  // green
    IM_COL32( 80, 140, 220, 255),  // blue
    IM_COL32(220, 160,  40, 255),  // amber
    IM_COL32(160,  80, 220, 255),  // purple
    IM_COL32( 40, 200, 200, 255),  // teal
    IM_COL32(220, 120, 200, 255),  // pink
    IM_COL32(200, 200,  40, 255),  // yellow
};
static constexpr int kNumFieldColors = sizeof(kFieldColors) / sizeof(kFieldColors[0]);

static ImU32 BlockTypeColor(EBufferBlockType type)
{
    switch (type)
    {
        case EBufferBlockType::Static:  return IM_COL32( 50, 110, 180, 255);
        case EBufferBlockType::Dynamic: return IM_COL32( 50, 155,  90, 255);
        case EBufferBlockType::Scratch: return IM_COL32(145,  75, 175, 255);
        default:                        return IM_COL32( 90,  90,  90, 255);
    }
}

static const char* BlockTypeName(EBufferBlockType type)
{
    switch (type)
    {
        case EBufferBlockType::Static:  return "Static";
        case EBufferBlockType::Dynamic: return "Dynamic";
        case EBufferBlockType::Scratch: return "Scratch";
        default:                        return "Unknown";
    }
}

static ImU32 LerpColor(ImU32 a, ImU32 b, float t)
{
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    ImVec4 ca = ImGui::ColorConvertU32ToFloat4(a);
    ImVec4 cb = ImGui::ColorConvertU32ToFloat4(b);
    return ImGui::ColorConvertFloat4ToU32({
        ca.x + (cb.x - ca.x) * t,
        ca.y + (cb.y - ca.y) * t,
        ca.z + (cb.z - ca.z) * t,
        ca.w + (cb.w - ca.w) * t });
}

static ImU32 DimColor(ImU32 color, float factor)
{
    ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
    return ImGui::ColorConvertFloat4ToU32({ c.x * factor, c.y * factor, c.z * factor, c.w });
}

// ── MemoryTool ─────────────────────────────────────────────────────────────────

MemoryTool::PendingData MemoryTool::BuildPending(const BlockBuffer& buffer, bool captureDirty)
{
    PendingData data;
    data.HasLayout = true;
    data.TotalSize = buffer.GetSize();

    const auto& blocks = buffer.GetBlocks();
    data.Blocks.reserve(blocks.size());

    for (size_t i = 0; i < blocks.size(); ++i)
    {
        const BlockBuffer::Block& b = blocks[i];

        BlockInfo info;
        if (b.Definition.Type)
            info.Name = b.Definition.Type->GetDisplayName().c_str();
        info.Offset     = static_cast<uint32_t>(b.Offset);
        info.HeaderSize = b.Definition.Layout.GetHeaderSize();
        info.AllocSize  = b.Definition.Layout.GetAllocSize();
        info.Size       = info.HeaderSize + info.AllocSize;
        info.BlockType  = static_cast<EBufferBlockType>(b.Definition.SortOrder);
        info.Type       = b.Definition.Type;
        info.Layout     = b.Definition.Layout;

        data.Blocks.push_back(std::move(info));
    }

    if (captureDirty)
    {
        const auto& pages = buffer.GetDirtyPageOffsets();
        data.DirtyPageOffsets.assign(pages.begin(), pages.end());
        data.DirtyPageSize = buffer.GetDirtyPageSize();
    }

    return data;
}

void MemoryTool::OnSimUpdate(WorldConstRef world, const Session& session)
{
    PendingData worldData = BuildPending(world.GetBuffer(), true);

    PendingData sessionData;
    if (!SessionLayoutCaptured)
    {
        sessionData = BuildPending(session.GetBuffer(), false);
        SessionLayoutCaptured = true;
    }

    {
        std::lock_guard lock(Mutex);
        PendingWorld = std::move(worldData);
        if (sessionData.HasLayout)
            PendingSession = std::move(sessionData);
        HasPending = true;
    }
}

void MemoryTool::ApplySnapshot(BufferView& view, const PendingData& data)
{
    if (!data.HasLayout)
        return;

    bool layoutChanged = (view.TotalSize != data.TotalSize ||
                          view.Blocks.size() != data.Blocks.size());
    if (layoutChanged)
    {
        view.TotalSize = data.TotalSize;
        view.Blocks.clear();
        view.Blocks.reserve(data.Blocks.size());
        for (const BlockInfo& info : data.Blocks)
            view.Blocks.push_back(info);
        view.DirtyPageAges.clear();
        view.LayoutReady = true;
        view.SelectedBlock = -1;  // reset selection when layout changes
    }

    view.DirtyPageSize = data.DirtyPageSize;

    for (uint32_t page : data.DirtyPageOffsets)
        view.DirtyPageAges[page] = 0.0f;
}

// ── Layout tree helpers ───────────────────────────────────────────────────────

static const char* FNameStr(Phoenix::FName name)
{
    if (name == Phoenix::FName::None) return nullptr;
    const char* entry = Phoenix::FName::GetNameEntry(name);
    return (entry && entry[0]) ? entry : nullptr;
}

// Context describing a rendered segmented bar — shared between DrawSegmentedBar
// and DrawLayoutTree so hover highlights map back to the correct pixels.
struct AllocBarCtx
{
    ImVec2   origin;       // top-left of the bar (for hover outline y-range)
    float    x0;           // left edge of the first child segment
    float    width;        // total pixel width available for segments
    float    barH;         // bar height in pixels
    uint32_t totalAlloc;   // total allocated bytes (denominator for proportional widths)
    float    normT;        // 0 = proportional width, 1 = equal width per non-zero child
};

static int CountNonZeroChildren(const BlockBufferLayout& layout)
{
    int n = 0;
    for (const auto& c : layout.GetChildren())
    {
        if (c.GetAllocSize() > 0 && FNameStr(c.GetName())) ++n;
    }
    return n;
}

static float ChildPixelWidth(uint32_t childAlloc, const AllocBarCtx& ctx, int numNonZero)
{
    const float propW  = ctx.totalAlloc > 0
        ? ctx.width * static_cast<float>(childAlloc) / static_cast<float>(ctx.totalAlloc)
        : 0.0f;
    const float equalW = numNonZero > 0 ? ctx.width / static_cast<float>(numNonZero) : 0.0f;
    return propW + (equalW - propW) * ctx.normT;
}

// Draw proportional/equal-width colored segments for the immediate children of a layout node.
static void DrawSegmentedBar(ImDrawList* dl, const AllocBarCtx& ctx, const BlockBufferLayout& layout)
{
    const auto& children   = layout.GetChildren();
    const int   numNonZero = CountNonZeroChildren(layout);
    if (numNonZero == 0) return;

    float cx = ctx.x0;
    for (int i = 0; i < static_cast<int>(children.size()); ++i)
    {
        const BlockBufferLayout& child = children[i];
        const uint32_t childAlloc = child.GetAllocSize();
        if (childAlloc == 0) continue;

        const float childW = ChildPixelWidth(childAlloc, ctx, numNonZero);
        if (childW < 1.0f) { cx += childW; continue; }

        const ImU32 col = kFieldColors[i % kNumFieldColors];
        dl->AddRectFilled({cx, ctx.origin.y}, {cx + childW - 1.0f, ctx.origin.y + ctx.barH}, col);

        // 1px dark separator on the right edge of each segment for readability
        dl->AddLine({cx + childW - 1.0f, ctx.origin.y},
                    {cx + childW - 1.0f, ctx.origin.y + ctx.barH},
                    IM_COL32(0, 0, 0, 120));

        if (childW > 28.0f)
        {
            const char* name = FNameStr(child.GetName());
            if (name)
            {
                dl->PushClipRect({cx + 2.0f, ctx.origin.y},
                                 {cx + childW - 2.0f, ctx.origin.y + ctx.barH}, true);
                const float ty = ctx.origin.y + (ctx.barH - ImGui::GetTextLineHeight()) * 0.5f;
                dl->AddText({cx + 3.0f, ty}, IM_COL32(255, 255, 255, 220), name);
                dl->PopClipRect();
            }
        }
        cx += childW;
    }
}

// Draw the recursive region tree. Hovering a row outlines its segment in barCtx.
// When a node is expanded, a sub-bar is drawn inline and recursion uses it as the new barCtx.
static void DrawLayoutTree(
    const char* idBase,
    const BlockBufferLayout& layout,
    int depth,
    int parentColorIdx,
    const AllocBarCtx& barCtx)
{
    ImDrawList* dl         = ImGui::GetWindowDrawList();
    const auto& children   = layout.GetChildren();
    const int   numNonZero = CountNonZeroChildren(layout);

    float pixelX = barCtx.x0;
    for (int i = 0; i < static_cast<int>(children.size()); ++i)
    {
        const BlockBufferLayout& child = children[i];
        const uint32_t childAlloc = child.GetAllocSize();
        if (childAlloc == 0) continue;

        const float childW = ChildPixelWidth(childAlloc, barCtx, numNonZero);

        const int   colorIdx = (depth == 0) ? i : parentColorIdx;
        const ImU32 rawCol   = kFieldColors[colorIdx % kNumFieldColors];
        const ImU32 col      = (depth == 0) ? rawCol : DimColor(rawCol, 0.65f);
        const ImVec4 cv      = ImGui::ColorConvertU32ToFloat4(col);

        const char* name        = FNameStr(child.GetName());
        if (!name) continue;

        const bool hasChildren = CountNonZeroChildren(child);

        char nodeId[128];
        snprintf(nodeId, sizeof(nodeId), "%s##lt%d_%d", name ? name : "?", depth, i);

        ImGui::PushID(depth * 64 + i);

        ImGui::ColorButton("##c", cv,
            ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker, {12.0f, 12.0f});
        ImGui::SameLine(0.0f, 4.0f);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        const bool open = ImGui::TreeNodeEx(nodeId, flags,
            "%s  (%u B)", name ? name : "(unnamed)", childAlloc);

        // Hover: only depth-0 items map correctly to barCtx (the main alloc bar)
        if (depth == 0 && ImGui::IsItemHovered() && childW >= 1.0f)
        {
            dl->AddRect({pixelX,          barCtx.origin.y},
                        {pixelX + childW, barCtx.origin.y + barCtx.barH},
                        IM_COL32(255, 255, 255, 200), 0.0f, 0, 2.0f);
        }

        ImGui::PopID();

        if (hasChildren && open)
        {
            DrawLayoutTree(idBase, child, depth + 1, colorIdx, barCtx);
            ImGui::TreePop();
        }

        pixelX += childW;
    }
}

// ── DrawBlockDetail ───────────────────────────────────────────────────────────

void MemoryTool::DrawBlockDetail(const char* barLabel, const BlockInfo& info, BufferView& view)
{
    const float avail = ImGui::GetContentRegionAvail().x;

    // ── Title ─────────────────────────────────────────────────────────────────
    char buf[256];
    snprintf(buf, sizeof(buf), "  %s  —  header %u B | alloc %u B",
             info.Name.empty() ? "(unnamed)" : info.Name.c_str(),
             info.HeaderSize, info.AllocSize);
    ImGui::TextDisabled("%s", buf);

    ImGui::SetNextItemWidth(140.0f);
    ImGui::SliderFloat("##detail_norm", &view.DetailNormT, 0.0f, 1.0f, "norm %.2f");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("0 = proportional to byte size\n1 = equal width per region");

    // ── Build sorted field list via reflection ────────────────────────────────
    struct FieldInfo
    {
        std::string Name;
        std::string TypeName;
        uint32_t Offset = 0;
        uint32_t Size   = 0;
    };
    std::vector<FieldInfo> fields;

    if (info.Type)
    {
        for (const auto& [name, fd] : info.Type->GetFields())
        {
            FieldInfo fi;
            fi.Name     = name;
            fi.Offset   = fd.GetOffset();
            fi.Size     = fd.GetType() ? fd.GetType()->GetSize() : 0;
            fi.TypeName = fd.GetType() ? fd.GetType()->GetDisplayName() : "?";
            fields.push_back(std::move(fi));
        }
        std::sort(fields.begin(), fields.end(),
                  [](const FieldInfo& a, const FieldInfo& b){ return a.Offset < b.Offset; });

        // Re-derive sizes from offset differences so padding is captured correctly.
        for (size_t i = 0; i + 1 < fields.size(); ++i)
            fields[i].Size = fields[i + 1].Offset - fields[i].Offset;
        if (!fields.empty() && info.HeaderSize > 0)
            fields.back().Size = info.HeaderSize - fields.back().Offset;
    }

    // ── Mini bar: header fields | alloc region ────────────────────────────────
    const ImVec2 barOrigin = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(barOrigin,
                      { barOrigin.x + avail, barOrigin.y + kDetailBarH },
                      IM_COL32(25, 25, 25, 255));

    const float totalSz  = static_cast<float>(info.Size);
    const float headerW  = (totalSz > 0 && info.Size > 0)
                               ? avail * static_cast<float>(info.HeaderSize) / totalSz
                               : avail;
    const float hPxPerB  = info.HeaderSize > 0 ? headerW / static_cast<float>(info.HeaderSize) : 0.0f;

    // AllocBarCtx is built once here and reused for both bar drawing and the tree below.
    AllocBarCtx allocCtx;
    allocCtx.origin     = barOrigin;
    allocCtx.x0         = barOrigin.x + headerW + 1.0f;
    allocCtx.width      = std::max(0.0f, avail - headerW - 1.0f);
    allocCtx.barH       = kDetailBarH;
    allocCtx.totalAlloc = info.AllocSize;
    allocCtx.normT      = view.DetailNormT;

    // Header: one colored segment per field
    for (size_t i = 0; i < fields.size(); ++i)
    {
        if (fields[i].Size == 0) continue;
        const float fx0 = barOrigin.x + static_cast<float>(fields[i].Offset) * hPxPerB;
        const float fx1 = barOrigin.x + static_cast<float>(fields[i].Offset + fields[i].Size) * hPxPerB;
        const ImU32 col = kFieldColors[i % kNumFieldColors];
        dl->AddRectFilled({ fx0, barOrigin.y }, { fx1 - 1.0f, barOrigin.y + kDetailBarH }, col);
        if (fx1 - fx0 > 28.0f)
        {
            dl->PushClipRect({ fx0 + 2.0f, barOrigin.y }, { fx1 - 2.0f, barOrigin.y + kDetailBarH }, true);
            const float ty = barOrigin.y + (kDetailBarH - ImGui::GetTextLineHeight()) * 0.5f;
            dl->AddText({ fx0 + 3.0f, ty }, IM_COL32(255, 255, 255, 220), fields[i].Name.c_str());
            dl->PopClipRect();
        }
    }

    if (!info.Type && info.HeaderSize > 0)
    {
        // No reflection — draw a dim placeholder for the header region
        dl->AddRectFilled(barOrigin,
                          { barOrigin.x + headerW, barOrigin.y + kDetailBarH },
                          IM_COL32(60, 60, 60, 200));
        dl->PushClipRect(barOrigin, { barOrigin.x + headerW, barOrigin.y + kDetailBarH }, true);
        const float ty = barOrigin.y + (kDetailBarH - ImGui::GetTextLineHeight()) * 0.5f;
        dl->AddText({ barOrigin.x + 4.0f, ty }, IM_COL32(140, 140, 140, 200), "no reflection data");
        dl->PopClipRect();
    }

    // Alloc region — segmented by layout children when available
    if (info.AllocSize > 0 && allocCtx.width > 1.0f)
    {
        if (CountNonZeroChildren(info.Layout))
        {
            DrawSegmentedBar(dl, allocCtx, info.Layout);
        }
        else
        {
            const float ax1 = barOrigin.x + avail;
            dl->AddRectFilled({ allocCtx.x0, barOrigin.y }, { ax1, barOrigin.y + kDetailBarH },
                              IM_COL32(75, 75, 75, 220));
            if (allocCtx.width > 36.0f)
            {
                dl->PushClipRect({ allocCtx.x0 + 2.0f, barOrigin.y },
                                 { ax1 - 2.0f, barOrigin.y + kDetailBarH }, true);
                const float ty = barOrigin.y + (kDetailBarH - ImGui::GetTextLineHeight()) * 0.5f;
                dl->AddText({ allocCtx.x0 + 4.0f, ty }, IM_COL32(190, 190, 190, 200), "alloc");
                dl->PopClipRect();
            }
        }
    }

    // ── Dirty page overlays on mini-bar ──────────────────────────────────────
    // The mini-bar has two sections with independent px-per-byte scales:
    //   [0, headerW)    ↔ block bytes [0, HeaderSize)
    //   [headerW, avail) ↔ block bytes [HeaderSize, Size)
    if (view.DirtyPageSize > 0 && !view.DirtyPageAges.empty())
    {
        constexpr ImU32 kDirtyFull  = IM_COL32(255, 120, 30, 220);
        constexpr ImU32 kDirtyFaded = IM_COL32(255, 120, 30,   0);

        const float allocW  = avail - headerW;
        const float aPxPerB = (info.AllocSize > 0 && allocW > 0.0f)
                                  ? allocW / static_cast<float>(info.AllocSize)
                                  : 0.0f;
        const uint32_t blockEnd = info.Offset + info.Size;

        for (const auto& [pageOff, age] : view.DirtyPageAges)
        {
            const uint32_t pageEnd = pageOff + view.DirtyPageSize;
            if (pageOff >= blockEnd || pageEnd <= info.Offset) continue;

            const ImU32 color = LerpColor(kDirtyFull, kDirtyFaded, age / kFadeDuration);
            const uint32_t localS = std::max(pageOff, info.Offset) - info.Offset;
            const uint32_t localE = std::min(pageEnd, blockEnd)    - info.Offset;

            // Header section
            if (localS < info.HeaderSize && hPxPerB > 0.0f)
            {
                const float px0 = barOrigin.x + static_cast<float>(localS) * hPxPerB;
                const float px1 = barOrigin.x + static_cast<float>(std::min(localE, info.HeaderSize)) * hPxPerB;
                dl->AddRectFilled({ px0, barOrigin.y },
                                  { px1 < px0 + 1.0f ? px0 + 1.0f : px1, barOrigin.y + kDetailBarH },
                                  color);
            }

            // Alloc section — mapped through normalized segment positions when children exist
            if (localE > info.HeaderSize)
            {
                const uint32_t allocS = std::max(localS, info.HeaderSize) - info.HeaderSize;
                const uint32_t allocE = localE - info.HeaderSize;

                const int numNonZero = CountNonZeroChildren(info.Layout);
                if (numNonZero > 0)
                {
                    float cx = allocCtx.x0;
                    for (const auto& child : info.Layout.GetChildren())
                    {
                        const uint32_t childAlloc = child.GetAllocSize();
                        if (childAlloc == 0 || !FNameStr(child.GetName())) continue;
                        const float    childW     = ChildPixelWidth(childAlloc, allocCtx, numNonZero);
                        const uint32_t childStart = child.GetAllocOffset();
                        const uint32_t childEnd   = childStart + childAlloc;
                        if (allocS < childEnd && allocE > childStart)
                        {
                            const float    pxPerByte = childW / static_cast<float>(childAlloc);
                            const uint32_t relS = std::max(allocS, childStart) - childStart;
                            const uint32_t relE = std::min(allocE, childEnd) - childStart;
                            const float px0 = cx + static_cast<float>(relS) * pxPerByte;
                            const float px1 = cx + static_cast<float>(relE) * pxPerByte;
                            dl->AddRectFilled({ px0, barOrigin.y },
                                { px1 < px0 + 1.0f ? px0 + 1.0f : px1, barOrigin.y + kDetailBarH }, color);
                        }
                        cx += childW;
                    }
                }
                else if (aPxPerB > 0.0f)
                {
                    const uint32_t as = std::max(localS, info.HeaderSize);
                    const float px0 = barOrigin.x + headerW + static_cast<float>(as - info.HeaderSize) * aPxPerB;
                    const float px1 = barOrigin.x + headerW + static_cast<float>(localE - info.HeaderSize) * aPxPerB;
                    dl->AddRectFilled({ px0, barOrigin.y },
                        { px1 < px0 + 1.0f ? px0 + 1.0f : px1, barOrigin.y + kDetailBarH }, color);
                }
            }
        }
    }

    ImGui::SetCursorScreenPos({ barOrigin.x, barOrigin.y + kDetailBarH });
    ImGui::Dummy({ avail, 0.0f });
    ImGui::Spacing();

    // ── Field table ──────────────────────────────────────────────────────────
    if (!fields.empty())
    {
        char tblId[64];
        snprintf(tblId, sizeof(tblId), "##%s_ft", barLabel);

        if (ImGui::BeginTable(tblId, 4,
                ImGuiTableFlags_SizingStretchProp |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Field",  ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type",   ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 58.0f);
            ImGui::TableSetupColumn("Size",   ImGuiTableColumnFlags_WidthFixed, 58.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < fields.size(); ++i)
            {
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(i));

                ImGui::TableSetColumnIndex(0);
                ImVec4 cv = ImGui::ColorConvertU32ToFloat4(kFieldColors[i % kNumFieldColors]);
                ImGui::ColorButton("##c", cv,
                    ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker,
                    { 10.0f, 10.0f });
                ImGui::SameLine(0.0f, 4.0f);
                ImGui::TextUnformatted(fields[i].Name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextDisabled("%s", fields[i].TypeName.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%u", fields[i].Offset);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%u B", fields[i].Size);

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
    else if (!info.Type)
    {
        ImGui::TextDisabled("  (no reflection data for header)");
    }

    // ── Alloc region tree ────────────────────────────────────────────────────
    if (!info.Layout.GetChildren().empty())
    {
        ImGui::Spacing();
        ImGui::TextDisabled("  Allocated regions:");
        ImGui::Spacing();
        ImGui::Indent(8.0f);
        DrawLayoutTree(barLabel, info.Layout, 0, 0, allocCtx);
        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();
}

// ── Flame-graph segment builder ───────────────────────────────────────────────

static void BuildSegmentsRecursive(
    std::vector<FlameGraphSegment>& segs,
    const BlockBufferLayout&        layout,
    uint64_t                        allocAbsStart,
    int                             depth,
    int                             parentColorIdx)
{
    const auto& children   = layout.GetChildren();
    const int   numNonZero = CountNonZeroChildren(layout);
    if (numNonZero == 0) return;

    for (int i = 0; i < static_cast<int>(children.size()); ++i)
    {
        const BlockBufferLayout& child     = children[i];
        const uint32_t           childSize = child.GetAllocSize();
        if (childSize == 0) continue;

        const int   colorIdx = (depth == 1) ? i : parentColorIdx;
        const float dimFactor = 1.0f - 0.12f * static_cast<float>(std::max(0, depth - 1));
        const ImU32 col = DimColor(kFieldColors[colorIdx % kNumFieldColors],
                                   std::max(0.4f, dimFactor));

        FlameGraphSegment seg;
        seg.Start = allocAbsStart + child.GetAllocOffset();
        seg.Size  = childSize;
        seg.Depth = depth;
        seg.Color = col;
        seg.Label = FNameStr(child.GetName());
        seg.Tag   = static_cast<uintptr_t>(-1);
        segs.push_back(seg);

        if (!child.GetChildren().empty())
            BuildSegmentsRecursive(segs, child, seg.Start, depth + 1, colorIdx);
    }
}

std::vector<FlameGraphSegment> MemoryTool::BuildBufferSegments(const BufferView& view)
{
    std::vector<FlameGraphSegment> segs;
    segs.reserve(view.Blocks.size() * 4);

    for (int i = 0; i < static_cast<int>(view.Blocks.size()); ++i)
    {
        const MemoryTool::BlockInfo& block = view.Blocks[i];
        if (block.Size == 0) continue;

        // Depth 0: whole block, split visually between header (dim) and alloc (bright).
        // We emit two segments if the block has both parts.
        const ImU32 baseColor = BlockTypeColor(block.BlockType);

        const char* blockLabel = block.Name.empty() ? nullptr : block.Name.c_str();

        if (block.HeaderSize > 0)
        {
            FlameGraphSegment hdr;
            hdr.Start = block.Offset;
            hdr.Size  = block.HeaderSize;
            hdr.Depth = 0;
            hdr.Color = DimColor(baseColor, 0.5f);
            hdr.Label = blockLabel;
            hdr.Tag   = static_cast<uintptr_t>(i);
            segs.push_back(hdr);
        }

        if (block.AllocSize > 0)
        {
            FlameGraphSegment alloc;
            alloc.Start = block.Offset + block.HeaderSize;
            alloc.Size  = block.AllocSize;
            alloc.Depth = 0;
            alloc.Color = baseColor;
            alloc.Label = blockLabel;  // label on both; clip rect handles overlap
            alloc.Tag   = static_cast<uintptr_t>(i);
            segs.push_back(alloc);

            if (!block.Layout.GetChildren().empty())
            {
                BuildSegmentsRecursive(segs, block.Layout,
                                       static_cast<uint64_t>(block.Offset + block.HeaderSize),
                                       1, i % kNumFieldColors);
            }
        }
    }

    return segs;
}

// ── DrawBufferBar ─────────────────────────────────────────────────────────────

void MemoryTool::DrawBufferBar(const char* label, BufferView& view, FlameGraphView& fgView)
{
    if (!view.LayoutReady || view.TotalSize == 0)
    {
        ImGui::TextDisabled("%s: (no data)", label);
        return;
    }

    const size_t numBlocks = view.Blocks.size();

    char header[128];
    snprintf(header, sizeof(header), "%s  —  %u B  (%zu blocks)",
             label, view.TotalSize, numBlocks);
    ImGui::TextDisabled("%s", header);

    if (view.DirtyPageSize > 0)
        ImGui::TextDisabled("scroll=zoom  right-drag=pan  click=select  dbl-click=frame  |  page = %u B",
                            view.DirtyPageSize);
    else
        ImGui::TextDisabled("scroll=zoom  right-drag=pan  click=select  dbl-click=frame");

    // Build flat segment list for this frame.
    std::vector<FlameGraphSegment> segs = BuildBufferSegments(view);

    // Dirty overlay — maps dirty page byte ranges onto segment pixel extents.
    constexpr ImU32 kDirtyFull  = IM_COL32(255, 120, 30, 220);
    constexpr ImU32 kDirtyFaded = IM_COL32(255, 120, 30,   0);

    FlameGraphOverlayFn overlayFn;
    if (view.DirtyPageSize > 0 && !view.DirtyPageAges.empty())
    {
        overlayFn = [&](const FlameGraphSegment& seg,
                        ImDrawList*              dl,
                        ImVec2                   rMin,
                        ImVec2                   rMax)
        {
            if (seg.Size == 0) return;
            const double pxPerByte = static_cast<double>(rMax.x - rMin.x)
                                   / static_cast<double>(seg.Size);
            for (const auto& [pageOff, age] : view.DirtyPageAges)
            {
                const uint64_t pageEnd = static_cast<uint64_t>(pageOff) + view.DirtyPageSize;
                if (pageEnd <= seg.Start || pageOff >= seg.Start + seg.Size) continue;
                const uint64_t relS = std::max<uint64_t>(pageOff, seg.Start) - seg.Start;
                const uint64_t relE = std::min<uint64_t>(pageEnd, seg.Start + seg.Size) - seg.Start;
                const float px0 = rMin.x + static_cast<float>(static_cast<double>(relS) * pxPerByte);
                const float px1 = rMin.x + static_cast<float>(static_cast<double>(relE) * pxPerByte);
                const ImU32 color = LerpColor(kDirtyFull, kDirtyFaded, age / kFadeDuration);
                dl->AddRectFilled({px0, rMin.y},
                    {px1 < px0 + 1.0f ? px0 + 1.0f : px1, rMax.y}, color);
            }
        };
    }

    // Click → select block for detail panel.
    FlameGraphClickFn clickFn = [&](const FlameGraphSegment& seg)
    {
        if (seg.Depth == 0)
        {
            const int blockIdx = static_cast<int>(seg.Tag);
            view.SelectedBlock = (blockIdx == view.SelectedBlock) ? -1 : blockIdx;
        }
    };

    char fgId[64];
    snprintf(fgId, sizeof(fgId), "##fg_%s", label);
    DrawFlameGraph(fgId, fgView, segs, view.TotalSize,
                   kBarH, overlayFn, clickFn);

    ImGui::Spacing();

    // ── Detail panel for selected block ───────────────────────────────────────
    if (view.SelectedBlock >= 0 && view.SelectedBlock < static_cast<int>(numBlocks))
    {
        ImGui::PushID(label);
        DrawBlockDetail(label, view.Blocks[view.SelectedBlock], view);
        ImGui::PopID();
    }
}

// ── DrawWindow ────────────────────────────────────────────────────────────────

void MemoryTool::Draw(float deltaTime)
{
    PendingData localWorld, localSession;
    bool hasNew = false;
    {
        std::lock_guard lock(Mutex);
        if (HasPending)
        {
            std::swap(localWorld, PendingWorld);
            std::swap(localSession, PendingSession);
            hasNew = true;
            HasPending = false;
        }
    }

    if (hasNew)
    {
        ApplySnapshot(WorldView, localWorld);
        ApplySnapshot(SessionView, localSession);
    }

    auto AdvanceAndPrune = [deltaTime](BufferView& view)
    {
        for (auto it = view.DirtyPageAges.begin(); it != view.DirtyPageAges.end(); )
        {
            it->second += deltaTime;
            it = (it->second >= kFadeDuration) ? view.DirtyPageAges.erase(it) : std::next(it);
        }
    };
    AdvanceAndPrune(WorldView);
    AdvanceAndPrune(SessionView);

    // ── Legend ────────────────────────────────────────────────────────────────
    {
        auto LegendItem = [](ImU32 color, const char* name)
        {
            ImVec4 c = ImGui::ColorConvertU32ToFloat4(color);
            ImGui::ColorButton(name, c,
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker,
                { 12.0f, 12.0f });
            ImGui::SameLine(0.0f, 4.0f);
            ImGui::TextUnformatted(name);
            ImGui::SameLine(0.0f, 14.0f);
        };

        LegendItem(IM_COL32(255, 120, 30, 220),                   "Dirty page (fades 0.5 s)");
        LegendItem(BlockTypeColor(EBufferBlockType::Static),        "Static alloc");
        LegendItem(DimColor(BlockTypeColor(EBufferBlockType::Static), 0.5f), "Static header");
        LegendItem(BlockTypeColor(EBufferBlockType::Dynamic),       "Dynamic alloc");
        LegendItem(DimColor(BlockTypeColor(EBufferBlockType::Dynamic), 0.5f), "Dynamic header");

        ImGui::NewLine();

        // Dirty page size on its own line so it's clearly readable.
        const uint32_t pageSize = WorldView.DirtyPageSize > 0 ? WorldView.DirtyPageSize
                                : SessionView.DirtyPageSize;
        if (pageSize > 0)
            ImGui::TextDisabled("  Dirty page size: %u B", pageSize);
    }

    ImGui::Separator();

    DrawBufferBar("World",   WorldView,   WorldFGView);
    DrawBufferBar("Session", SessionView, SessionFGView);
}
