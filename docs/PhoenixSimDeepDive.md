# PhoenixSim Engine Deep-Dive

## Core Structure & Architecture
- **PhoenixSim** is the core simulation library for high-performance, modular simulation.
- Organized around **Session** and **World**:
  - **Session**: Manages global state, configuration, services, features, and the main simulation buffer.
  - **World**: Isolated simulation context with its own buffer, config, and random state. Managed by `WorldManager`.

## Key Classes
- **Session**: Handles initialization, shutdown, ticking, stepping, action queue, buffer management. Owns `FeatureSet`, `WorldManager`, `ServiceContainer`.
- **WorldManager**: Creates/manages worlds, steps them, routes actions.
- **FeatureSet/IFeature**: Modular simulation features (ECS, blackboard, physics) that define buffer blocks and participate in layout.
- **BlockBuffer**: Contiguous memory buffer for simulation data, central to cache coherency.
- **ServiceContainer/IService**: Dependency injection for services (config, logging).

## Buffer Architecture & Cache Coherency
- **SessionBuffer**: Allocated during session creation, layout determined by registered features.
- **World Buffer**: Each world has its own buffer, layout defined by features and config.
- **BlockBufferLayoutBuilder**: Used by features/services to register blocks, ensuring efficient packing.
- **Cache Coherency**: Contiguous buffers and block registration minimize cache misses and false sharing.
