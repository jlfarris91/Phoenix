# PhoenixSteering

PhoenixSteering handles unit movement — pathfinding to goals, following other entities, collision avoidance, and facing/rotation. It works by driving `BodyComponent::LinearVelocity` each frame, with `PhoenixPhysics` handling the actual position integration and collision response.

**Key files:** `src/PhoenixSteering/FeatureSteering.h`, `SteeringComponent.h`, `SteeringSystem.h`

---

## SteeringComponent

Add a `SteeringComponent` to any entity that needs to move under steering control. It is automatically included on unit entities spawned by `FeatureUnit`.

The component tracks:
- Current movement target (position or entity)
- Movement state (moving, turning, holding, arrived)
- Speed configuration (max speed, turn rate)

---

## Movement API

```cpp
// Move to a world-space position, stopping within `range` of the target
FeatureSteering::MoveToLocation(world, entityId, targetPos, range);

// Follow another entity, staying within `range`
FeatureSteering::FollowEntity(world, entityId, targetEntityId, range);

// Cancel movement
FeatureSteering::Stop(world, entityId);

// Query movement state
bool moving   = FeatureSteering::IsMoving(entityId);
bool arrived  = FeatureSteering::HasFinishedMoving(entityId);
bool holding  = FeatureSteering::IsHolding(entityId);  // immovable state

// Modify speed at runtime (e.g. apply a slow debuff)
FeatureSteering::UpdateSpeed(world, entityId, speedArgs);
```

---

## Rotation API

Steering handles facing separately from movement. A unit can turn in place without moving.

```cpp
// Face a world-space position
FeatureSteering::TurnToFace(world, entityId, targetPos);

// Face another entity
FeatureSteering::TurnToFace(world, entityId, targetEntityId);

// Query rotation state
bool turning  = FeatureSteering::IsTurning(entityId);
bool done     = FeatureSteering::HasFinishedTurning(entityId);
```

---

## Spatial Queries

`FeatureSteering` maintains a frame-sorted spatial index of all steerable entities, which other systems can query cheaply:

```cpp
auto nearby = FeatureSteering::QueryEntitiesInRange(world, pos, radius, args);
for (auto* entry : nearby)
{
    EntityId id  = entry->Id;
    float    dist = entry->Distance;
    // ...
}
```

This is used internally for collision avoidance and is available to gameplay code for things like proximity checks and area-of-effect targeting.

---

## How Steering Works

Each tick, `SteeringSystem`:

1. **Sorts entities** by Morton Z-code for spatial locality
2. **Computes a desired velocity** for each moving entity — seeks toward the goal position at max speed, slowing within the arrival radius
3. **Computes avoidance** — queries nearby entities and adds repulsion forces to prevent overlap
4. **Writes `LinearVelocity`** into `BodyComponent` — PhoenixPhysics integrates this and resolves collisions
5. **Updates facing** — rotates the entity's transform toward the movement direction or explicit turn target at the configured turn rate

The result is smooth, decentralized unit movement without a global pathfinder. For structured path following (waypoints, formation movement), issue sequential `MoveToLocation` calls through the orders system.

---

## Relationship to PhoenixPhysics

Steering and physics cooperate rather than compete:

- **Steering** sets `LinearVelocity` (intent) and marks the body `EBodyMovement::Moving`
- **Physics** integrates that velocity, applies damping, and resolves penetration constraints
- When a unit is `Stop()`ped, steering zeroes the velocity and marks the body `EBodyMovement::Idle`, allowing physics damping to bring it to rest naturally
