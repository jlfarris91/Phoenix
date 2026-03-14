# PhoenixSim Core

PhoenixSim is the foundation every other library builds on. It provides the session/world lifecycle, the Feature system, an archetype-based ECS, a game data layer (LDS), pre-allocated block buffers, and supporting utilities (hashing, threading, containers).

---

## Session and World

Everything lives inside a `Session`. A session owns:

- A `FeatureSet` — all registered features
- A `WorldManager` — one or more simulation worlds
- A `ServiceContainer` — dependency-injected services
- A session-level `BlockBuffer` — pre-allocated memory for session-scoped feature data
- An `ActionQueue` — thread-safe command inbox

A `World` is an isolated simulation context. Each world has its own `BlockBuffer` and ECS state. Multiple worlds can run inside a single session — separate lobbies, replay playback alongside live simulation, etc.

**Key files:** `src/PhoenixSim/Session.h`, `src/PhoenixSim/Worlds.h`

### Lifecycle

```
Session::Create(args)
    → features register session/world block layouts
    → session buffer allocated
Session::Initialize()
    → features initialized
    → worlds created, world buffers allocated
    → ECS systems registered

Session::Tick(args)          ← called each game loop iteration
    → process enqueued actions
    → for each world:
        → Feature::OnPreWorldUpdate / OnWorldUpdate / OnPostWorldUpdate
        → ECS systems execute

Session::Shutdown()
```

---

## Feature System

A Feature is a pluggable subsystem. Every major capability in PhoenixSim and its dependent libraries is implemented as a Feature. See [FeatureDevelopmentWorkflow.md](FeatureDevelopmentWorkflow.md) for a full guide on writing one.

The built-in features in PhoenixSim are:

| Feature | Purpose |
|---|---|
| `FeatureECS` | Entity/component management, system runner |
| `FeatureLDS` | Game data catalogs |
| `FeatureBlackboard` | Per-frame cross-feature shared state |

**Key file:** `src/PhoenixSim/Features.h`

### Execution order

Features execute in channel order each tick:

```
OnPreWorldUpdate  →  OnWorldUpdate  →  OnPostWorldUpdate
```

Features declare their position relative to others at registration time using `FeatureInsertPosition::Before` / `After`.

---

## ECS

PhoenixSim uses an archetype-based ECS. See [ECS.md](ECS.md) for the full guide. In brief:

- **`EntityId`** — 32-bit handle, non-reusable
- **`IComponent`** — plain data struct, declared with `PHX_ECS_DECLARE_COMPONENT`
- **`TransformComponent`** — always present on every entity (position, parent, Morton Z-code)
- **`ArchetypeList`** — contiguous storage for all entities sharing the same component set
- **`EntityQuery`** — filter by `WithAll / WithNone / WithAllTags / WithNoneTags`
- **`ForEachEntity(world, query, lambda)`** — the primary iteration API
- **`ISystem`** — encapsulates recurring per-world entity logic; registered in `OnWorldInitialize`

**Key files:** `src/PhoenixSim/ECS/ArchetypeManager.h`, `ECS/FeatureECS.h`, `ECS/EntityQuery.h`, `ECS/System.h`

---

## LDS (Layered Data System)

LDS is the game data layer — a hierarchical catalog of typed definitions loaded from JSON.

Data is organized into two halves:
- **Types** — shared templates (e.g., a `Marine` unit type with default stats)
- **Objects** — instances that inherit from and override types

A property lookup walks up the inheritance chain to the nearest definition. This lets you define defaults at the type level and override selectively at the object level.

At runtime, `Data::UnitPtr`, `Data::WeaponPtr`, etc. are type-safe lazy-evaluated pointers into the catalog.

**Key files:** `src/PhoenixSim/LDS/FeatureLDS.h`, `LDS/LDSCatalog.h`

---

## BlockBuffer and Memory

All persistent simulation data lives in pre-allocated `BlockBuffer`s. See [WorldAndSessionBuffers.md](WorldAndSessionBuffers.md) for a full guide. In brief:

- **Session buffer** — global, allocated once
- **World buffer** — one per world
- Block types: `Static` (fixed), `Dynamic` (growable), `Scratch` (cleared each frame)
- Access: `world.GetBlockRef<MyBlock>()`
- Never use `std::vector` / `std::map` in blocks — use the `Fixed*` containers in `src/PhoenixSim/Containers/`

**Key file:** `src/PhoenixSim/Containers/BlockBuffer.h`

---

## FName and Hashing

`FName` is a 32-bit FNV1A hash of a string, used everywhere as a type-safe identifier — entity kinds, component IDs, data object names, feature names, property keys. Cheap to copy and compare.

```cpp
FName name("Marine");
FName combined = name.Append("_Veteran");
```

In debug builds (`PHOENIX_SIM_NAME_ENTRIES` defined), recover the original string with `FName::GetNameEntry(hash)`.

**Key files:** `src/PhoenixSim/Name.h`, `src/PhoenixSim/Hashing.h`

---

## Blackboard

The Blackboard is a per-frame key-value store for sharing data between features within a single tick. It is cleared at the start of each world update — nothing written to it persists across frames.

```cpp
// Writer
auto& bb = FeatureBlackboard::GetBlackboard(world);
bb.Set(FName("ProjectileHit"), hitEvent);

// Reader (same frame, later in execution order)
if (auto* hit = bb.Get<ProjectileHitEvent>(FName("ProjectileHit")))
{
    // react to hit
}
```

**Key file:** `src/PhoenixSim/Blackboard/FeatureBlackboard.h`

---

## Threading

PhoenixSim has a `ThreadPool` for background work. The simulation tick is single-threaded by default; the pool is used for parallelizable workloads (spatial sorting, broad-phase physics).

```cpp
SetThreadPool("main", numWorkers);
auto handle = GetThreadPool()->Submit([] { /* work */ });
handle->Wait();

// Distribute loop iterations:
ParallelForEach(count, [](uint32 i) { /* work */ });
```

**Key file:** `src/PhoenixSim/Parallel.h`

---

## Platform Macros

Cross-platform helpers are in `src/PhoenixSim/Platform.h`:

| Macro | Purpose |
|---|---|
| `PHX_DEBUG_BREAK()` | `__debugbreak()` on MSVC, `__builtin_trap()` elsewhere |
| `PHX_FORCEINLINE` | `__forceinline` on MSVC, `__attribute__((always_inline))` elsewhere |
| `PHX_THREAD_PAUSE()` | `_mm_pause()` on MSVC, no-op elsewhere |
| `sprintf_s(...)` | Maps to `snprintf` on non-MSVC |

Always use these instead of the platform-specific intrinsics directly — the codebase builds for both Windows (MSVC) and WebAssembly (Emscripten/clang).
