# PhoenixSim — Claude Reference

This file is loaded automatically by Claude Code. It contains architecture notes and conventions that inform how to work on this codebase.

---

## Module Stack

```
PhoenixLua          (scripting bridge — depends on all)
PhoenixRTS          (units, abilities, orders, effects, vitals, projectiles)
PhoenixSteering     (pathfinding and unit steering)
PhoenixPhysics      (2D rigid-body physics)
PhoenixSim          (core: ECS, Session, Features, LDS, memory, threading)
```

## Core Patterns

**Feature** — primary extension point. Subclass `IFeature`, macro `PHX_DECLARE_FEATURE_TYPE`. Hooks: `OnSessionLayout`, `OnWorldLayout`, `OnWorldInitialize`, `OnWorldUpdate`. Features expose a static `Get(world)` + static API by convention.

**BlockBuffer** — all simulation data lives in pre-allocated buffers. Session buffer (global) + per-World buffer. Features register `Static` / `Dynamic` / `Scratch` blocks in layout hooks. Access via `world.GetBlockRef<T>()`. Never put `std::vector`/`std::map` in a block — use the `Fixed*` containers from `src/PhoenixSim/Containers/`.

**ECS** — archetype-based. `EntityId` (32-bit handle), `IComponent` (plain data struct), `ArchetypeList` (contiguous storage per archetype). `EntityQueryBuilder::WithAll/WithNone/WithAllTags`. `FeatureECS::ForEachEntity(world, query, lambda)`. `ISystem` with `OnPreWorldUpdate/OnWorldUpdate/OnPostWorldUpdate`. `TransformComponent` is always present on every entity.

**FName** — 32-bit FNV1A hash of a string. Used everywhere as an identifier. `FName("text")`, `.Append(str)`. String recovery via `FName::GetNameEntry()` in debug builds only.

**LDS** — hierarchical game data layer. Types (templates) + Objects (instances). Inheritance-chain property lookup. `Data::UnitPtr`, `Data::WeaponPtr`, etc. are lazy-evaluated pointers into the catalog.

**Blackboard** — per-frame key-value store for cross-feature communication within a tick. Cleared at the start of each world update. `FeatureBlackboard::GetBlackboard(world).Set/Get`.

## Docs

| Doc | What it covers |
|---|---|
| [FeatureDevelopmentWorkflow.md](docs/FeatureDevelopmentWorkflow.md) | How to write a Feature — start here |
| [ECS.md](docs/ECS.md) | Entities, components, queries, systems |
| [WorldAndSessionBuffers.md](docs/WorldAndSessionBuffers.md) | Block buffer memory model |
| [PhoenixSimDeepDive.md](docs/PhoenixSimDeepDive.md) | Session, World, LDS, threading, FName |
| [PhoenixPhysics.md](docs/PhoenixPhysics.md) | BodyComponent, PhysicsSystem, force API |
| [PhoenixSteering.md](docs/PhoenixSteering.md) | Movement, rotation, spatial queries |
| [PhoenixRTS.md](docs/PhoenixRTS.md) | Units, abilities, orders, effects, vitals, projectiles |
| [BuildAndRunTestRTS.md](docs/BuildAndRunTestRTS.md) | Build setup and running TestRTS |
| [ScriptingBindings.md](docs/ScriptingBindings.md) | Runtime-agnostic scripting binding architecture (IScriptRuntime / IScriptBindings) |
| [Reflection.md](docs/Reflection.md) | TypeDescriptor, properties, methods, macros, TypeRegistry, serialization, script bindings |
| [WasmScriptingInternals.md](docs/WasmScriptingInternals.md) | wasm3 stack convention, Emscripten STANDALONE_WASM, invoke_* dispatch, file-based Lua loading, diagnostics |
| [ThreadingModel.md](docs/ThreadingModel.md) | Sim / game / render thread split, WorldDoubleBuffer as sim→game bridge, SessionInstance::Tick / GetWorldView |
| [SceneLayer.md](docs/SceneLayer.md) | Phoenix.Scene + Phoenix.Scene.EnTT: SceneEntitySync, IEntitySyncHandler, EnTTScene, engine-agnostic design |

## Key Files

| Area | File |
|---|---|
| Session | `src/PhoenixSim/Session.h` |
| Features | `src/PhoenixSim/Features.h` |
| ECS manager | `src/PhoenixSim/ECS/ArchetypeManager.h` |
| ECS feature | `src/PhoenixSim/ECS/FeatureECS.h` |
| Entity query | `src/PhoenixSim/ECS/EntityQuery.h` |
| System base | `src/PhoenixSim/ECS/System.h` |
| LDS | `src/PhoenixSim/LDS/FeatureLDS.h`, `LDS/LDSCatalog.h` |
| Block buffer | `src/PhoenixSim/Containers/BlockBuffer.h` |
| Threading | `src/PhoenixSim/Parallel.h` |
| Name/hash | `src/PhoenixSim/Name.h` |
| Platform macros | `src/PhoenixSim/Platform.h` |
| Physics | `src/PhoenixPhysics/FeaturePhysics.h`, `PhysicsSystem.h`, `BodyComponent.h` |
| Steering | `src/PhoenixSteering/FeatureSteering.h` |
| Units | `src/PhoenixRTS/Units/FeatureUnit.h` |
| Abilities | `src/PhoenixRTS/Abilities/FeatureAbilities.h` |
| Orders | `src/PhoenixRTS/Orders/FeatureOrders.h` |
| Effects | `src/PhoenixRTS/Effects/FeatureEffects.h` |
| Vitals | `src/PhoenixRTS/Vitals/FeatureVitals.h` |
| Unit data types | `src/PhoenixRTS/Data/DataUnit.h` (100+ types in same folder) |

## PhoenixRTS Systems

- **FeatureUnit**: `SpawnUnit` / `ReleaseUnit` / `QueryUnitsInRange`. Unit = ECS entity + `UnitComponent` (owner + data ref).
- **FeatureAbilities**: `RegisterAbilityHandler(IAbilityHandler)`. `AddAbility` / `HasAbility` / `GetAbilities`.
- **FeatureOrders**: `RegisterCommandHandler(ICommandHandler)`. `IssueCommand` → `EnqueueOrder`. Per-unit `FixedOrderQueue`. `OnActiveOrderCompleted` advances queue.
- **FeatureEffects**: `AcquireEffectScope` (source/target context). `IEffectHandler` executes, `IResponseHandler` reacts. Double-buffered deferred effects.
- **FeatureVitals**: `ApplyDamage(world, target, amount)`. Handles death detection.
- **FeatureProjectile**: Spawn / track / impact.

## PhoenixPhysics

- `BodyComponent`: Force, LinearVelocity, MaxLinearVelocity, LinearDamping, InvMass, Radius, `EBodyMovement` (Idle/Moving/Attached), `EBodyFlags` (Awake/StaticX/StaticY).
- `PhysicsSystem` phases: `OnPreWorldUpdate` (Z-code spatial sort) → `OnWorldUpdate` (integrate, collide, solve) → `OnPostWorldUpdate` (sleep management).
- API: `QueryEntitiesInRange`, `AddExplosionForceToEntitiesInRange`, `AddForce`.

## Build

- **Build system**: CMake + vcpkg. Presets: `windows` → `.build/windows`, `emscripten` → `.build/wasm`.
- **TestRTS** is gated by `PHX_BUILD_TESTAPP`. Dependencies (SDL3, imgui, tracy) are in the `testapp` vcpkg feature — not installed for Emscripten builds.
- **No `<execution>`** — `#include <execution>` and `std::execution::par` cause MSVC OOM on CI runners. Use sequential `std::sort`.
- **Platform macros** in `Platform.h`: `PHX_DEBUG_BREAK()`, `PHX_FORCEINLINE`, `PHX_THREAD_PAUSE()`, `sprintf_s` compat for non-MSVC.
- **CI**: Two workflows — `build-windows.yml` and `build-emscripten.yml`. Both triggered on `pull_request`. Push to `main` goes through `release-please.yml` which calls both. Release Please manages changelog and versioning.

## Conventions

- Features expose functionality as `static void DoSomething(WorldRef, ...)` — callers never hold feature pointers.
- `FName` is the universal ID type. Prefer it over raw strings and integer IDs.
- ECS queries are built once in `OnWorldInitialize` and stored as members, not rebuilt each frame.
- Structural ECS changes (add/remove component) are deferred — don't assume the change takes effect within the same frame.
