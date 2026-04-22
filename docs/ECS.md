# ECS Guide

PhoenixSim uses an archetype-based ECS. Entities with the same set of components are stored together in contiguous arrays (`ArchetypeList`), which makes iterating a filtered set of entities very cache-friendly.

---

## Entities

An `EntityId` is a lightweight 32-bit handle. You never construct one directly — the ECS allocates them.

```cpp
// Acquire a new entity
EntityId id = FeatureECS::AcquireEntity(world, "MyKind");

// Release (does not immediately free — deferred until end of frame)
FeatureECS::ReleaseEntity(world, id);
```

`"MyKind"` is an `FName` that categorizes the entity. It's used in queries to filter by entity type.

---

## Components

A component is a plain data struct. Declare it with the macro:

```cpp
// MyComponent.h
#include "PhoenixSim/ECS/Component.h"

struct MyComponent : public Phoenix::ECS::IComponent
{
    PHX_ECS_DECLARE_COMPONENT(MyComponent)

    int   Health = 100;
    float Speed  = 1.0f;
};
```

Add and retrieve components on an entity:

```cpp
// Add
MyComponent* comp = FeatureECS::AddComponent<MyComponent>(world, entityId);
comp->Health = 200;

// Get (returns nullptr if not present)
MyComponent* comp = FeatureECS::GetComponent<MyComponent>(world, entityId);

// Get or add
MyComponent* comp = FeatureECS::GetOrAddComponent<MyComponent>(world, entityId);

// Remove
FeatureECS::RemoveComponent<MyComponent>(world, entityId);
```

When you add or remove a component, the entity moves to a different `ArchetypeList`. This is deferred to the end of the frame — don't assume the entity is in its new archetype until the next tick.

---

## Querying Entities

`EntityQuery` lets you filter by component presence. Build one with the fluent builder:

```cpp
auto query = Phoenix::ECS::EntityQueryBuilder()
    .WithAll<TransformComponent, MyComponent>()   // must have both
    .WithNone<DeadComponent>()                    // must not have this
    .Build();
```

Then iterate matching entities:

```cpp
FeatureECS::ForEachEntity(world, query,
    [](Phoenix::ECS::EntityId id,
       Phoenix::ECS::TransformComponent& transform,
       MyComponent& my)
    {
        my.Health -= 1;
        // transform.Transform.Position is the entity's position
    });
```

The component types in the lambda must be a subset of `WithAll<>` — the ECS uses them to resolve pointers per entity.

For read-only access, take the component by `const&`.

---

## Systems

A System encapsulates recurring per-world logic. Prefer systems over putting logic directly in `OnWorldUpdate` when the logic touches many entities or needs the full ECS iteration machinery.

```cpp
// MySystem.h
#include "PhoenixSim/ECS/System.h"

class MySystem : public Phoenix::ECS::ISystem
{
    PHX_ECS_DECLARE_SYSTEM(MySystem)

public:
    void OnWorldInitialize(Phoenix::WorldRef world) override;
    void OnWorldUpdate(Phoenix::WorldRef world,
                       const Phoenix::ECS::SystemUpdateArgs& args) override;
    void OnWorldShutdown(Phoenix::WorldRef world) override;

private:
    Phoenix::ECS::EntityQuery m_Query;
};
```

```cpp
// MySystem.cpp
void MySystem::OnWorldInitialize(Phoenix::WorldRef world)
{
    m_Query = Phoenix::ECS::EntityQueryBuilder()
        .WithAll<TransformComponent, MyComponent>()
        .Build();
}

void MySystem::OnWorldUpdate(Phoenix::WorldRef world,
                              const Phoenix::ECS::SystemUpdateArgs& args)
{
    FeatureECS::ForEachEntity(world, m_Query,
        [&](Phoenix::ECS::EntityId id, TransformComponent& t, MyComponent& my)
        {
            // per-entity logic
        });
}
```

Register the system in your feature's `OnWorldInitialize`:

```cpp
void MyFeature::OnWorldInitialize(Phoenix::WorldRef world)
{
    auto sys = std::make_shared<MySystem>();
    FeatureECS::GetECS(world)->RegisterSystem(sys);
}
```

### System lifecycle hooks

| Hook | When it runs |
|---|---|
| `OnWorldInitialize` | Once when the world is created |
| `OnPreWorldUpdate` | Each tick, before `OnWorldUpdate` — use for sorting/prep |
| `OnWorldUpdate` | Each tick — main logic |
| `OnPostWorldUpdate` | Each tick, after `OnWorldUpdate` — cleanup/deferred work |
| `OnWorldShutdown` | Once when the world is destroyed |

---

## Tags

Tags are zero-size markers on entities. They work like components in queries but carry no data:

```cpp
// Declare
struct AliveTag : public Phoenix::ECS::IComponent
{
    PHX_ECS_DECLARE_COMPONENT(AliveTag)
};

// Apply
FeatureECS::AddTag<AliveTag>(world, entityId);

// Query — use WithAllTags / WithNoneTags
auto query = Phoenix::ECS::EntityQueryBuilder()
    .WithAll<TransformComponent>()
    .WithAllTags<AliveTag>()
    .WithNoneTags<DeadTag>()
    .Build();
```

---

## TransformComponent

Every entity has a `TransformComponent` automatically — you don't need to add it. It contains:

```cpp
struct TransformComponent : IComponent
{
    EntityId    AttachParent;   // parent entity (or invalid)
    Transform2D Transform;      // position + rotation
    uint32      ZCode;          // Morton code for spatial sorting (managed by ECS)
};
```

Access position:
```cpp
auto* t = FeatureECS::GetComponent<TransformComponent>(world, id);
auto pos = t->Transform.Position;  // Phoenix::Math::Vec2
```

---

## Jobs and Parallel Scheduling

The job system is the primary way to process entities in parallel. Jobs participate in a DAG-based scheduler that serializes conflicting jobs and runs independent ones concurrently.

### IJob — per-entity job

Subclass `IJob<TComponents...>` where `TComponents` is the list of component types the job needs. Each entity is processed by a single `Execute()` call:

```cpp
struct MyJob : Phoenix::ECS::IJob<TransformComponent&, const MyComponent&>
{
    FName GetName() const override { return "MyJob"_n; }

    void Execute(WorldConstRef world, EntityId id, CommandBuffer& cb,
                 TransformComponent& transform,
                 const MyComponent& my) override
    {
        if (my.ShouldRelease)
            cb.Append<Commands::ReleaseEntity>(id);
    }
};
```

Component types follow the same rules as `ForEachEntity` — `T&` for read-write, `const T&` for read-only. The scheduler uses these access modes to detect conflicts between jobs.

Optional batch hooks run once per matching archetype, outside the entity loop:

```cpp
void BeginBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) override;
void EndBatch(WorldConstRef world, const JobBatch& batch, CommandBuffer& cb) override;
```

### ITask — single-execution task

`ITask` participates in the same DAG as jobs but runs once per scheduler execution rather than once per entity. Use it for setup/teardown that must be ordered relative to `IJob` work:

```cpp
struct MySortTask : Phoenix::ECS::ITask
{
    void Run(WorldConstRef world, CommandBuffer& cb) override
    {
        std::sort(...);
    }
};
```

### Registering jobs in the global scheduler

Register jobs from `ISystem::OnWorldInitialize`. The global scheduler runs automatically each tick — **before** system `OnXxxWorldUpdate` methods:

```cpp
void MySystem::OnWorldInitialize(WorldRef world)
{
    JobHandle hA = FeatureECS::RegisterJob(world,
        std::make_unique<MyJobA>(), EJobPhase::Update);

    JobHandle hB = FeatureECS::RegisterJob(world,
        std::make_unique<MyJobB>(), EJobPhase::Update);

    // B must not start until A has finished
    FeatureECS::AddJobDependency(world, EJobPhase::Update, /*after=*/hB, /*before=*/hA);
}
```

`EJobPhase` controls when the scheduler fires relative to the world tick:

| Phase | Runs before |
|---|---|
| `PreUpdate` | `ISystem::OnPreWorldUpdate` |
| `Update` | `ISystem::OnWorldUpdate` |
| `PostUpdate` | `ISystem::OnPostWorldUpdate` |

### System-owned schedulers

The global scheduler runs **before** system update methods, so globally-registered jobs cannot receive per-frame data (DeltaTime, config values) that the system sets during its own update. For those cases, own the `JobScheduler` on the system and drive it manually:

```cpp
class MySystem : public ISystem
{
    void OnWorldInitialize(WorldRef world) override
    {
        auto job = std::make_unique<MyJob>();
        JobPtr = job.get();
        Scheduler.RegisterJob(std::move(job));
        // call Scheduler.AddDependency(...) as needed
        // Build() is called internally by ExecuteScheduler on first run
    }

    void OnWorldUpdate(WorldRef world, const SystemUpdateArgs& args) override
    {
        JobPtr->DeltaTime = args.DeltaTime;     // inject per-frame data before running
        FeatureECS::ExecuteScheduler(world, Scheduler);
    }

    ECS::JobScheduler Scheduler;
    MyJob* JobPtr = nullptr;   // raw ptr for per-frame param updates; Scheduler owns lifetime
};
```

`FeatureECS::ExecuteScheduler` runs the caller-owned scheduler using the world's thread pool and command buffers, and rebuilds archetype batches automatically when archetypes change.

### CommandBuffer — deferring mutations

Each `Execute()` call receives a per-thread `CommandBuffer`. Use it to defer any mutation that touches shared or cross-entity state:

```cpp
// Structural changes
cb.Append<Commands::ReleaseEntity>(id);
cb.Append<Commands::AddTag<AliveTag>>(id);

// Construct-in-place from args
cb.Append<Commands::SetBlackboardValue<float>>(id, "Speed"_n, 2.5f);

// From a pre-constructed value
cb.Append(Commands::SetEntityKind{id, "NewKind"_n});
```

Command buffers are flushed and applied serially after the parallel job phase completes. Register a handler to react to custom commands:

```cpp
FeatureECS::RegisterCommandHandler<MyCommand>(world,
    [](WorldRef world, const MyCommand& cmd) { /* ... */ });
```

### What is and isn't safe from Execute()

| Operation | Safe in parallel? | How |
|---|---|---|
| Read component args | Yes | Each entity owns distinct memory |
| Write component args | Yes | Each entity's slot is exclusive |
| Read feature/world block data | Yes | Treat as a read-only snapshot during job execution |
| Mutate shared or global state | **No** | Use `CommandBuffer` |
| Call feature statics that mutate | **No** | Use `CommandBuffer` |
| Acquire or release entities | **No** | Use `CommandBuffer` |

---

## Practical Tips

**Don't add/remove components inside `ForEachEntity`** — structural changes are deferred, but modifying the archetype list you're currently iterating causes undefined behavior. Queue the changes and apply them after the loop.

**Build queries once** — construct `EntityQuery` in `OnWorldInitialize` and store it as a member. Building a query every frame allocates and compares component lists unnecessarily.

**Use `WithNone<>` aggressively** — excluding dead, dormant, or cargo entities from a query is cheaper than checking inside the loop.

**Component access is O(1)** — `GetComponent<T>` does a direct offset lookup into the archetype block. It's fast but not free in tight loops; prefer getting a pointer once outside the loop if you need it multiple times.
