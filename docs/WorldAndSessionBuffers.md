# World and Session Buffers in PhoenixSim

## Overview
PhoenixSim enforces cache coherency and data locality through its buffer architecture. Simulation data is stored in contiguous memory blocks, minimizing cache misses and maximizing performance.

## Session Buffer
- **SessionBuffer**: Allocated during session creation.
- **Layout**: Determined by registered features via `BlockBufferLayoutBuilder`.
- **Purpose**: Stores global simulation state, feature data, and configuration.
- **Access**: Use `Session::GetBuffer()` to access session-level data.

## World Buffer
- **World Buffer**: Each world has its own buffer, allocated on creation.
- **Layout**: Defined by features and world configuration.
- **Purpose**: Stores per-world simulation state, entities, and feature data.
- **Access**: Use `World::GetBuffer()` to access world-level data.

## Buffer Layout & Registration
- Features/services register blocks in `OnSessionLayout`/`OnWorldLayout`.
- `BlockBufferLayoutBuilder` ensures all blocks are packed efficiently.
- Example registration:
  ```cpp
  builder.RegisterBlock(blockDefinition);
  builder.RegisterBlockWithAlloc<FeatureBlock>(EBufferBlockType::Dynamic, blockConfig);
  ```

## Cache Coherency
- Contiguous buffers reduce false sharing and cache misses.
- Data is accessed via block offsets, ensuring predictable memory access.
- Double buffering (e.g., `DoubleWorldBuffer` in TestRTS) allows safe concurrent simulation and rendering.

## Diagram
```
[Session]
+-------------------+
| SessionBuffer     |
+-------------------+
      |
      v
[WorldManager]
+-------------------+
| World1: BlockBuffer|
| World2: BlockBuffer|
| ...               |
+-------------------+
```

## Best Practices
- Register all persistent data as buffer blocks.
- Avoid scattered allocations; use block buffers for simulation-critical data.
- Document block layouts for maintainability.
