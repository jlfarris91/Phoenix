# Feature Development Workflow in PhoenixSim

## Overview
PhoenixSim is designed for modular feature development. Features are implemented as subclasses of `IFeature`, allowing you to extend simulation functionality (ECS, blackboard, physics, etc.) in a cache-coherent, data-driven way.

## Steps to Add a New Feature

1. **Create a Feature Class**
   - Subclass `IFeature` (or a relevant base feature).
   - Implement required virtual methods: `OnSessionLayout`, `OnWorldLayout`, `OnUpdate`, etc.

2. **Register Buffer Blocks**
   - In `OnSessionLayout`/`OnWorldLayout`, use `BlockBufferLayoutBuilder` to register blocks for your feature's data.
   - This ensures your data is packed efficiently in the session/world buffer.

3. **Implement Update Logic**
   - Add simulation logic in `OnUpdate`, `OnWorldUpdate`, and other lifecycle hooks.
   - Use buffer accessors to read/write your feature's data.

4. **Register Feature in ServiceContainerBuilder**
   - In your app (e.g., `InitSession()` in TestRTS), register your feature:
     ```cpp
     serviceContainerBuilder->RegisterService<YourFeature>().AsInterfaces();
     ```

5. **Build and Run**
   - Rebuild the project and run TestRTS to see your feature in action.

## Example: Minimal Feature Skeleton
```cpp
class FeatureExample : public IFeature {
public:
    void OnSessionLayout(const SessionLayoutContext& context, BlockBufferLayoutBuilder& builder) override {
        // Register session-level blocks
    }
    void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder) override {
        // Register world-level blocks
    }
    void OnUpdate(const FeatureUpdateArgs& args) override {
        // Simulation logic
    }
};
```

## Best Practices
- Keep data structures simple and contiguous for cache efficiency.
- Use buffer blocks for all persistent simulation data.
- Leverage lifecycle hooks for initialization, updates, and shutdown.
- Document your feature's purpose and buffer layout.
