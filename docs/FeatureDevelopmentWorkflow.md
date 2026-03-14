# Writing a Feature

Features are the primary way to add new systems to PhoenixSim. A Feature is a class that:
- Declares what memory it needs (block registrations)
- Hooks into session and world lifecycle events
- Optionally registers ECS systems

This guide walks through writing one from scratch.

---

## 1. Declare the feature class

```cpp
// src/MyLib/MyFeature.h
#pragma once
#include "PhoenixSim/Features.h"
#include "PhoenixSim/ECS/FeatureECS.h"

class MYLIBRARY_API MyFeature : public Phoenix::IFeature
{
    PHX_DECLARE_FEATURE_TYPE(MyFeature)

public:
    // --- called at session creation time ---
    void OnSessionLayout(
        const Phoenix::SessionLayoutContext& ctx,
        Phoenix::BlockBufferLayoutBuilder& builder) override;

    // --- called at world creation time ---
    void OnWorldLayout(
        const Phoenix::WorldLayoutContext& ctx,
        Phoenix::BlockBufferLayoutBuilder& builder) override;

    void OnWorldInitialize(Phoenix::WorldRef world) override;
    void OnWorldShutdown(Phoenix::WorldRef world) override;

    // --- called every tick ---
    void OnWorldUpdate(
        Phoenix::WorldRef world,
        const Phoenix::FeatureUpdateArgs& args) override;

    // --- static helpers that other features/systems call ---
    static void DoSomething(Phoenix::WorldRef world, ...);
};
```

`PHX_DECLARE_FEATURE_TYPE(MyFeature)` registers the type name so the feature can be looked up by name at runtime.

---

## 2. Declare your data blocks

Features store their data in pre-allocated `BlockBuffer` blocks rather than on the heap. There are three block types:

| Type | Use when |
|---|---|
| `Static` | Size is fully known at layout time and never changes |
| `Dynamic` | Needs an allocator; can grow (e.g. entity maps, queues) |
| `Scratch` | Temporary data rebuilt each frame (e.g. sorted entity lists) |

Declare your block struct:

```cpp
// Session-level data (one instance for the whole session)
struct MyFeatureSessionBlock : Phoenix::BufferBlockBase
{
    int GlobalCounter = 0;
};

// World-level data (one instance per world)
struct MyFeatureWorldBlock : Phoenix::BufferBlockBase
{
    Phoenix::FixedMap<Phoenix::ECS::EntityId, MyState, 1024> StateByEntity;
    // ^ capacity passed as template arg for static blocks,
    //   or as constructor arg for dynamic blocks
};

// Scratch block (cleared at the start of each frame)
struct MyFeatureScratch : Phoenix::BufferBlockBase
{
    Phoenix::FixedArray<Phoenix::ECS::EntityId, 2048> SortedEntities;
};
```

Register them in the layout hooks:

```cpp
void MyFeature::OnSessionLayout(
    const Phoenix::SessionLayoutContext&,
    Phoenix::BlockBufferLayoutBuilder& builder)
{
    builder.RegisterBlock<MyFeatureSessionBlock>(
        Phoenix::EBufferBlockType::Static);
}

void MyFeature::OnWorldLayout(
    const Phoenix::WorldLayoutContext& ctx,
    Phoenix::BlockBufferLayoutBuilder& builder)
{
    // For a dynamic block that needs constructor args:
    builder.RegisterBlockWithAlloc<MyFeatureWorldBlock>(
        Phoenix::EBufferBlockType::Dynamic,
        ctx.Config.MaxEntities);   // passed to the block constructor

    builder.RegisterBlock<MyFeatureScratch>(
        Phoenix::EBufferBlockType::Scratch);
}
```

Access your blocks anywhere you have a `WorldRef` or `SessionRef`:

```cpp
auto& worldBlock = world.GetBlockRef<MyFeatureWorldBlock>();
auto& scratch    = world.GetBlockRef<MyFeatureScratch>();
auto& sessionBlock = session.GetBlockRef<MyFeatureSessionBlock>();
```

---

## 3. Register ECS systems

If your feature needs to iterate entities, create a `System` and register it in `OnWorldInitialize`:

```cpp
void MyFeature::OnWorldInitialize(Phoenix::WorldRef world)
{
    auto sys = std::make_shared<MySystem>();
    Phoenix::FeatureECS::GetECS(world)->RegisterSystem(sys);
}
```

See [ECS.md](ECS.md) for how to write the system.

---

## 4. Implement the update hook

```cpp
void MyFeature::OnWorldUpdate(
    Phoenix::WorldRef world,
    const Phoenix::FeatureUpdateArgs& args)
{
    auto& block = world.GetBlockRef<MyFeatureWorldBlock>();
    float dt = args.DeltaTime;

    // Access another feature:
    auto* steering = Phoenix::FeatureSteering::Get(world);
    // (each Feature exposes a static Get(world) helper by convention)

    // ... your logic
}
```

---

## 5. Expose a static API

By convention, features expose their functionality as static methods so callers don't need to hold a pointer:

```cpp
// In MyFeature.h
static void DoSomething(Phoenix::WorldRef world, Phoenix::ECS::EntityId entity);

// In MyFeature.cpp
void MyFeature::DoSomething(Phoenix::WorldRef world, Phoenix::ECS::EntityId entity)
{
    auto* feature = Get(world);   // Get() resolves the feature from the world's session
    auto& block   = world.GetBlockRef<MyFeatureWorldBlock>();
    // ...
}
```

Callers use it as:
```cpp
MyFeature::DoSomething(world, entityId);
```

---

## 6. Register the feature in session setup

Features are registered when building the `ServiceContainer` (typically in your app's session setup, or in a game module initializer):

```cpp
Phoenix::ServiceContainerBuilder builder;
builder.RegisterService<MyFeature>().AsInterfaces<Phoenix::IFeature>();
// ... register other features
auto session = Phoenix::Session::Create({ .Services = builder.Build() });
```

Order matters: features that depend on others should be registered after their dependencies (or use `FeatureInsertPosition` to declare explicit ordering).

---

## Full lifecycle reference

| Hook | Session or World | When it runs |
|---|---|---|
| `OnSessionLayout` | Session | Before session buffer is allocated |
| `OnSessionInitialize` | Session | After session buffer is allocated |
| `OnWorldLayout` | World | Before world buffer is allocated |
| `OnWorldInitialize` | World | After world buffer is allocated; register ECS systems here |
| `OnPreUpdate` | Session | Each tick, before world updates |
| `OnUpdate` | Session | Each tick |
| `OnPostUpdate` | Session | Each tick, after world updates |
| `OnPreWorldUpdate` | World | Each tick, before world systems run |
| `OnWorldUpdate` | World | Each tick — main logic |
| `OnPostWorldUpdate` | World | Each tick, after world systems run; deferred cleanup |
| `OnWorldShutdown` | World | World is being destroyed |
| `OnSessionShutdown` | Session | Session is being destroyed |
| `OnPreHandleAction` | Session | Before an action is dispatched |
| `OnHandleAction` | Session | Handle a session-level action |
| `OnHandleWorldAction` | World | Handle a world-level action |

Most features only need `OnWorldLayout`, `OnWorldInitialize`, and `OnWorldUpdate`.

---

## Common patterns

### Accessing another feature

```cpp
auto* physics = Phoenix::FeaturePhysics::Get(world);
if (physics)
{
    physics->AddForce(world, entityId, force);
}
```

### Using the Blackboard for cross-feature communication within a frame

```cpp
// Writer (runs first in the frame)
auto& bb = Phoenix::FeatureBlackboard::GetBlackboard(world);
bb.Set(FName("SomeEvent"), myValue);

// Reader (runs later in the frame)
auto& bb = Phoenix::FeatureBlackboard::GetBlackboard(world);
if (auto val = bb.Get<MyValueType>(FName("SomeEvent")))
{
    // use *val
}
```

The blackboard is cleared at the start of each world update.

### Registering a handler (abilities, effects, orders, responses)

```cpp
void MyFeature::OnWorldInitialize(Phoenix::WorldRef world)
{
    auto* abilities = Phoenix::FeatureAbilities::Get(world);
    abilities->RegisterAbilityHandler(
        std::make_shared<MyAbilityHandler>());
}
```

### Accessing LDS data

```cpp
Phoenix::Data::UnitPtr unitData = Phoenix::FeatureUnit::GetUnitData(world, unitId);
float speed = unitData->Movement->MaxSpeed();
```
