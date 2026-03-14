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

## Practical Tips

**Don't add/remove components inside `ForEachEntity`** — structural changes are deferred, but modifying the archetype list you're currently iterating causes undefined behavior. Queue the changes and apply them after the loop.

**Build queries once** — construct `EntityQuery` in `OnWorldInitialize` and store it as a member. Building a query every frame allocates and compares component lists unnecessarily.

**Use `WithNone<>` aggressively** — excluding dead, dormant, or cargo entities from a query is cheaper than checking inside the loop.

**Component access is O(1)** — `GetComponent<T>` does a direct offset lookup into the archetype block. It's fast but not free in tight loops; prefer getting a pointer once outside the loop if you need it multiple times.
