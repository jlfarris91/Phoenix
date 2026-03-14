# PhoenixPhysics

PhoenixPhysics is a 2D rigid-body physics simulation integrated with the PhoenixSim ECS. It is position-based with iterative constraint solving and Morton-code spatial sorting for efficient broad-phase collision detection.

**Key files:** `src/PhoenixPhysics/FeaturePhysics.h`, `PhysicsSystem.h`, `BodyComponent.h`

---

## BodyComponent

Attach a `BodyComponent` to any ECS entity to make it participate in the physics simulation.

```cpp
struct BodyComponent : IComponent
{
    PHX_ECS_DECLARE_COMPONENT(BodyComponent)

    Vec2  Force;
    Vec2  LinearVelocity;
    float MaxLinearVelocity;
    float LinearDamping;
    float InvMass;           // 0 = infinite mass (static body)
    float Radius;            // collision radius (circle collider)

    EBodyMovement Movement;  // Idle, Moving, Attached
    EBodyFlags    Flags;     // Awake, StaticX, StaticY
};
```

**`EBodyFlags`:**
- `Awake` — body is active in the simulation; sleeping bodies are skipped
- `StaticX` / `StaticY` — constrain movement along an axis

**`EBodyMovement`:**
- `Idle` — not moving
- `Moving` — actively being driven (by steering or direct force)
- `Attached` — pinned to a parent entity

---

## PhysicsSystem

`PhysicsSystem` runs in three phases each tick:

### OnPreWorldUpdate — spatial sort
All physics entities are sorted by their Morton Z-code (derived from `TransformComponent::ZCode`). This gives spatial locality for the broad-phase sweep.

### OnWorldUpdate — simulate
1. **Integration** — apply forces, update velocities, integrate positions
2. **Broad phase** — sweep sorted entities, generate candidate contact pairs by radius overlap
3. **Narrow phase** — refine contacts, compute penetration depth and contact normal
4. **Constraint solving** — iterative position correction over `NumSolverSteps` iterations
5. **Sleep management** — bodies below velocity threshold accumulate `SleepTimer`

### OnPostWorldUpdate — cleanup
Retire contacts, advance sleep timers, put eligible bodies to sleep.

**Configurable parameters** (on `PhysicsSystem`):
- `NumIterations` — outer solver iterations
- `NumSolverSteps` — inner constraint steps per iteration
- `PenetrationThreshold` — minimum overlap before a contact is generated

---

## FeaturePhysics API

### Spatial queries

```cpp
// Find all physics entities within a radius
auto hits = FeaturePhysics::QueryEntitiesInRange(world, pos, range);
for (auto* entity : hits) { ... }
```

### Force application

```cpp
// Apply an impulse to a single entity
FeaturePhysics::AddForce(world, entityId, forceVec);

// Apply an explosion force to all entities within a radius
FeaturePhysics::AddExplosionForceToEntitiesInRange(
    world, epicenter, radius, strength);
```

---

## Adding Physics to an Entity

When spawning an entity that should participate in physics, add a `BodyComponent` and set its properties:

```cpp
EntityId id = FeatureECS::AcquireEntity(world, "Projectile");

auto* body = FeatureECS::AddComponent<BodyComponent>(world, id);
body->Radius             = 0.5f;
body->InvMass            = 1.0f / 10.0f;   // mass = 10
body->MaxLinearVelocity  = 20.0f;
body->LinearDamping      = 0.1f;
body->Flags              = EBodyFlags::Awake;

// Give it an initial velocity
body->LinearVelocity = direction * speed;
```

The physics system picks it up automatically on the next tick.

---

## Integration with Steering

`PhoenixSteering` drives units by setting `BodyComponent::LinearVelocity` and `EBodyMovement::Moving` each frame. The physics system then integrates that velocity and resolves collisions. The two systems cooperate: steering sets intent, physics enforces constraints.
