# PhoenixRTS

PhoenixRTS is the RTS gameplay layer built on top of PhoenixSim, PhoenixPhysics, and PhoenixSteering. It provides units, abilities, a command/order queue, a game effects system, vitals (health/shields), and projectiles — all driven by a data layer loaded from JSON catalogs.

---

## Data Layer

Before anything else, understand the data layer — it drives almost every system in PhoenixRTS.

All game data is defined in the LDS catalog as typed objects. At runtime you access it through type-safe pointer wrappers:

```cpp
Data::UnitPtr    unit    = FeatureUnit::GetUnitData(world, unitId);
Data::WeaponPtr  weapon  = unit->Weapons[0]();
Data::EffectPtr  effect  = weapon->Effects[0]();
```

**`DataUnit`** (`src/PhoenixRTS/Data/DataUnit.h`) is the root data structure for a unit type. It holds nested data for every aspect of the unit:

| Field | Type | What it is |
|---|---|---|
| `Actor` | `UnitActorPtr` | Visual representation |
| `Armor` | `UnitArmor` | Armor type and damage reduction |
| `Movement` | `UnitMovement` | MaxSpeed, TurnRate, AccelerationRadius |
| `Vision` | `UnitVision` | SightRange, DetectionRange |
| `Weapons` | `vector<WeaponPtr>` | All weapons on this unit |
| `Buffs` | `vector<BuffPtr>` | Passive buffs applied on spawn |
| `Tags` | `vector<TagPtr>` | Gameplay classification tags |
| `Info` | `UnitInfo` | Display name, description |
| `Death` | `UnitDeathStats` | What happens on death |
| `Build` | `UnitBuildStats` | Build time, cost |
| `Cargo` | `UnitCargoStats` | Transport capacity |
| … | … | 20+ more nested types |

The data layer for other objects (`DataAbility`, `DataEffect`, `DataWeapon`, `DataProjectile`, `DataBuff`, `DataResponse`, etc.) follows the same pattern — all in `src/PhoenixRTS/Data/`.

---

## Units

**Key file:** `src/PhoenixRTS/Units/FeatureUnit.h`

A unit is an ECS entity with a `UnitComponent` attached. `UnitComponent` stores the owning player index and an `FName` reference back to the unit's data object in the LDS catalog. Everything else about the unit — its physics body, steering state, abilities, vitals — lives in other components.

### Spawning

```cpp
UnitId id = FeatureUnit::SpawnUnit(
    world,
    unitDataName,   // FName — must exist in the LDS catalog
    ownerIndex,     // player/team index
    position,       // Vec2 world-space spawn position
    facing,         // initial rotation
    args            // optional SpawnArgs (custom component overrides, etc.)
);
```

`SpawnUnit` creates the ECS entity, attaches all required components, and calls `AddAbilitiesFromData` to populate the unit's ability set from its data definition.

### Querying

```cpp
// Find units within a radius
UnitQueryArgs queryArgs;
queryArgs.Flags     = EUnitQueryFlags::Alive;
queryArgs.Team      = ETeamFilter::Enemies;
queryArgs.MaxResults = 10;

auto nearby = FeatureUnit::QueryUnitsInRange(world, pos, range, queryArgs);
```

### Unit state checks

```cpp
FeatureUnit::IsUnitAlive(world, unitId);
FeatureUnit::IsUnitDead(world, unitId);
FeatureUnit::IsUnitHidden(world, unitId);
FeatureUnit::UnitCanMove(world, unitId);
FeatureUnit::UnitCanTurn(world, unitId);
FeatureUnit::UnitIsImmobilized(world, unitId);
FeatureUnit::GetOwningPlayer(world, unitId);  // → int
FeatureUnit::GetOwningTeam(world, unitId);    // → FName
```

---

## Abilities

**Key file:** `src/PhoenixRTS/Abilities/FeatureAbilities.h`

Abilities are named capabilities on a unit (e.g., `"Attack"`, `"Move"`, `"Blink"`). Each ability type has a registered `IAbilityHandler` that implements its logic.

### Registering a handler

```cpp
class MyAbilityHandler : public IAbilityHandler
{
    FName GetAbilityId() const override { return FName("MyAbility"); }
    bool  CanUse(WorldRef world, UnitId unit) override { ... }
    bool  Execute(WorldRef world, UnitId unit, const AbilityArgs& args) override { ... }
};

// In your feature's OnWorldInitialize:
FeatureAbilities::Get(world)->RegisterAbilityHandler(
    std::make_shared<MyAbilityHandler>());
```

### Managing abilities on units

```cpp
FeatureAbilities::AddAbility(world, unitId, FName("MyAbility"));
FeatureAbilities::RemoveAbility(world, unitId, FName("MyAbility"));
FeatureAbilities::HasAbility(world, unitId, FName("MyAbility"));

// Load all abilities defined in the unit's data:
FeatureAbilities::AddAbilitiesFromData(world, unitId, unitData);

// Get all ability IDs on a unit:
auto abilities = FeatureAbilities::GetAbilities(world, unitId);
```

---

## Orders and Commands

**Key file:** `src/PhoenixRTS/Orders/FeatureOrders.h`

The orders system separates player *intent* (commands) from unit *execution* (orders):

- **Command** — what the player asked for (e.g., "Attack this target"). Converted to an Order by a `ICommandHandler`.
- **Order** — a concrete unit directive that executes over time until complete.
- **OrderQueue** — per-unit FIFO queue of pending orders.

### Issuing a command

```cpp
AttackCommand cmd;
cmd.Target = targetUnitId;
FeatureOrders::IssueCommand(world, unitId, cmd);
// → ICommandHandler converts this to an AttackOrder and enqueues it
```

### Registering a command handler

```cpp
class MyCommandHandler : public ICommandHandler
{
    FName GetCommandId() const override { return FName("MyCommand"); }
    void  Handle(WorldRef world, UnitId unit,
                 const Command& cmd, OrderQueue& queue) override
    {
        MyOrder order;
        order.Target = static_cast<const MyCommand&>(cmd).Target;
        queue.Enqueue(order);
    }
};

FeatureOrders::Get(world)->RegisterCommandHandler(
    std::make_shared<MyCommandHandler>());
```

### Order execution

Each tick, `FeatureOrders` calls the active order's handler. When the order is done:

```cpp
FeatureOrders::OnActiveOrderCompleted(world, unitId, /*success=*/true);
// → dequeues the current order and starts the next one
```

### Inspecting the queue

```cpp
int count = FeatureOrders::GetNumOrders(world, unitId);

FeatureOrders::ForEachOrder(world, unitId, [](const Order& order) {
    // inspect
});

FeatureOrders::ClearOrderQueue(world, unitId);
```

---

## Effects and Responses

**Key file:** `src/PhoenixRTS/Effects/FeatureEffects.h`

The effects system is an event-dispatch pipeline for gameplay consequences — damage, healing, knockback, projectile launches, etc. It is intentionally decoupled: effect *execution* and effect *response* are registered independently.

### EffectScope

An `EffectScope` is the execution context for a batch of effects. It captures source and target:

```cpp
EffectScopeArgs scopeArgs;
scopeArgs.Source       = attackerUnitId;
scopeArgs.Target       = targetUnitId;
scopeArgs.SourcePos    = attackerPos;
scopeArgs.TargetPos    = targetPos;

auto scopeId = FeatureEffects::AcquireEffectScope(world, scopeArgs);
```

### Executing effects

```cpp
// Add individual effect nodes to the scope
FeatureEffects::AcquireEffectNode(world, scopeId, FName("Damage"));
FeatureEffects::AcquireEffectNode(world, scopeId, FName("LaunchProjectile"));

// Effects execute at the end of the frame (double-buffered deferred execution)
```

### Registering handlers

```cpp
class MyEffectHandler : public IEffectHandler
{
    FName GetEffectId() const override { return FName("MyEffect"); }
    void  Execute(WorldRef world, const EffectScope& scope,
                  const EffectNode& node) override { ... }
};

class MyResponseHandler : public IResponseHandler
{
    FName GetResponseId() const override { return FName("MyResponse"); }
    void  Respond(WorldRef world, const EffectScope& scope) override { ... }
};

auto* effects = FeatureEffects::Get(world);
effects->RegisterEffectHandler(std::make_shared<MyEffectHandler>());
effects->RegisterResponseHandler(std::make_shared<MyResponseHandler>());
```

Built-in handlers: `EffectDamageHandler`, `EffectLaunchProjectileHandler`, `EffectSetHandler` (executes multiple effects), `ResponseDamageHandler`.

---

## Vitals

**Key file:** `src/PhoenixRTS/Vitals/FeatureVitals.h`

Vitals track health, shields, and similar quantities. They are configured per unit type in the data layer and loaded into components at spawn.

```cpp
// Apply damage to a unit (goes through armor reduction defined in data)
FeatureVitals::ApplyDamage(world, targetUnitId, damageAmount);
```

The vitals system handles:
- Armor-type damage reduction (via unit's `UnitArmor` data)
- Death detection and triggering death events
- Shield regeneration (if defined in data)

Damage is almost always dealt via the effects system (`EffectDamageHandler` calls `ApplyDamage` internally) rather than directly, so responses fire correctly.

---

## Projectiles

**Key file:** `src/PhoenixRTS/Projectiles/FeatureProjectile.h`

Projectiles are physics-driven ECS entities. They are typically spawned by `EffectLaunchProjectileHandler` in response to a weapon attack effect.

```cpp
ProjectileArgs args;
args.Data      = FName("Bullet");   // LDS data object
args.Source    = attackerId;
args.Target    = targetId;
args.Origin    = firingPos;
args.Direction = aimDir;

ProjectileId pid = FeatureProjectile::SpawnProjectile(world, args);
```

`FeatureProjectile` moves the projectile each tick via its physics body and tests for impact. On impact it acquires an `EffectScope` and executes the projectile's on-hit effects (defined in its data).

---

## How It All Fits Together

A typical weapon attack flow:

```
Player issues "Attack" command
    → ICommandHandler converts to AttackOrder, enqueues it
    → FeatureOrders ticks AttackOrder each frame
        → checks weapon cooldown (FeatureTimers)
        → checks range (FeatureUnit::QueryUnitsInRange / FeatureSteering)
        → when in range and off cooldown:
            → AcquireEffectScope(attacker → target)
            → AcquireEffectNode("WeaponFire")
    → EffectWeaponFireHandler executes:
        → AcquireEffectNode("LaunchProjectile")
    → EffectLaunchProjectileHandler executes:
        → FeatureProjectile::SpawnProjectile(...)
    → Projectile travels, hits target
        → AcquireEffectScope(projectile → target)
        → AcquireEffectNode("Damage")
    → EffectDamageHandler executes:
        → FeatureVitals::ApplyDamage(target, amount)
    → VitalsSystem detects death
        → triggers death events → unit removed
```
