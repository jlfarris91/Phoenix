
#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <PhoenixSim/WorldsFwd.h>

#include "PhoenixSim/BlockBuffer/BlockBufferEnums.h"
#include "PhoenixSim/BlockBuffer/BlockBufferLayout.h"

#include "FlameGraph.h"

namespace Phoenix
{
    class BlockBuffer;
    class Session;
    class TypeDescriptor;
}

struct MemoryTool
{
    // Called from the sim thread after each world step.
    void OnSimUpdate(Phoenix::WorldConstRef world, const Phoenix::Session& session);

    // Called from the render thread each frame.
    void Draw(float deltaTime);

private:
    struct BlockInfo
    {
        std::string Name;
        uint32_t Offset = 0;
        uint32_t Size = 0;        // HeaderSize + AllocSize
        uint32_t HeaderSize = 0;  // sizeof the block struct
        uint32_t AllocSize = 0;   // extra allocation region after the struct
        Phoenix::EBufferBlockType BlockType = Phoenix::EBufferBlockType::Static;
        const Phoenix::TypeDescriptor* Type = nullptr;
        Phoenix::BlockBufferLayout Layout; // full recursive layout tree
    };

    struct BufferView
    {
        uint32_t TotalSize = 0;
        uint32_t DirtyPageSize = 0;
        std::vector<BlockInfo> Blocks;
        std::unordered_map<uint32_t, float> DirtyPageAges;  // pageOffset → age (seconds)
        bool LayoutReady = false;
        int SelectedBlock = -1;
        float DetailNormT = 0.0f;  // 0 = proportional, 1 = equal width per region
    };

    struct PendingData
    {
        bool HasLayout = false;
        uint32_t TotalSize = 0;
        std::vector<BlockInfo> Blocks;
        std::vector<uint32_t> DirtyPageOffsets;
        uint32_t DirtyPageSize = 0;
    };

    std::mutex Mutex;
    PendingData PendingWorld;
    PendingData PendingSession;
    bool HasPending = false;
    bool SessionLayoutCaptured = false;  // sim-thread only

    BufferView WorldView;
    BufferView SessionView;

    FlameGraphView WorldFGView;
    FlameGraphView SessionFGView;

    static PendingData BuildPending(const Phoenix::BlockBuffer& buffer, bool captureDirty);
    static void ApplySnapshot(BufferView& view, const PendingData& data);
    static std::vector<FlameGraphSegment> BuildBufferSegments(const BufferView& view);
    static void DrawBufferBar(const char* label, BufferView& view, FlameGraphView& fgView);
    static void DrawBlockDetail(const char* barLabel, const BlockInfo& info, BufferView& view);
};
