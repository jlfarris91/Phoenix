# Threading Model

PhoenixSim separates simulation, game logic, and rendering into three distinct threads. This document describes what runs on each thread, how data moves between them, and the invariants each layer depends on.

---

## The Three Threads

```
┌─────────────────────────────────────────────────────────────────┐
│  Sim Thread                                                      │
│  World::Update() — ECS, physics, steering, orders, effects      │
│  → WorldDoubleBuffer::OnSimUpdate(world)                        │
└────────────────────────────────┬────────────────────────────────┘
                                 │  dirty-page sync (O(dirty pages), not O(buffer size))
┌────────────────────────────────▼────────────────────────────────┐
│  Game Thread                                                     │
│  SessionInstance::Tick()       — advance all WorldDoubleBuffers  │
│  SceneEntitySync::Sync(world)  — spawn/update/despawn engine     │
│                                  entities from sim entities      │
│  EnTTScene::Gather(world)      — collect RenderScene primitives  │
└────────────────────────────────┬────────────────────────────────┘
                                 │  RenderScene (plain data, no World ref)
┌────────────────────────────────▼────────────────────────────────┐
│  Render Thread                                                   │
│  IRenderer::Submit(RenderScene) — SDL3, Unreal, Unity, etc.     │
│  No concept of World, Session, or sim state                      │
└─────────────────────────────────────────────────────────────────┘
```

The render thread never touches a `World`. It only ever sees a `RenderScene` — a flat list of `RenderCommand` draw calls with no simulation state.

---

## WorldDoubleBuffer

`src/Phoenix.Runtime/Worlds/WorldDoubleBuffer.h`

`WorldDoubleBuffer` is the **sim→game bridge** for one world. It is not a rendering concept. It holds two copies of the world buffer and swaps them between threads with a mutex held for nanoseconds (pointer swap only — no memcpy under the lock).

| Call | Thread | What it does |
|---|---|---|
| `OnSimUpdate(world)` | Sim | Syncs dirty pages into `SimView`; signals a new frame is ready |
| `Sink()` | Game | Swaps `SimView` and `WorldView` if a new frame is ready |
| `GetWorldView()` | Game | Returns the stable `const World*` for the current game frame |

`Sink()` is O(1) — no copy happens here, only pointer swaps. The dirty-page copy happens inside `OnSimUpdate` on the sim thread. See [WorldViewSync.md](WorldViewSync.md) for the full dirty-page strategy.

**One `WorldDoubleBuffer` per world per session.** `SessionInstance` owns a map of `FName → WorldDoubleBuffer` (keyed by world ID) and manages them internally. New buffers are created lazily on the first sim update for each world.

---

## WorldInstance

`src/Phoenix.Runtime/Worlds/WorldInstance.h`

`WorldInstance` owns the `WorldDoubleBuffer` for one world and exposes the game-thread interface for it. Parallel to `SessionInstance` for sessions.

```cpp
FName             GetId() const;
void              OnSimUpdate(WorldConstRef world);  // called by SessionInstance internals
void              Sink();                            // game thread — advance the buffer
const World*      GetWorldView() const;              // game thread — stable read view
double            GetUpdateRate() const;
uint32            GetAccumulatedDirtyPageCount() const;
```

`SessionInstance` owns `unordered_map<FName, unique_ptr<WorldInstance>>` and creates instances lazily on the first sim update for each world.

## SessionInstance Game-Thread API

`src/Phoenix.Runtime/Sessions/SessionInstance.h`

```cpp
// Called on game thread each frame — advances all WorldInstances for this session.
void Tick();

// Returns the WorldInstance for a given world ID, or null if not yet created.
WorldInstance* GetWorldInstance(FName worldId) const;

// Convenience: returns the stable world view for a given world ID, or null if not yet ready.
const Phoenix::World* GetWorldView(FName worldId) const;
```

`Tick()` iterates all `Worlds` and calls `world.Sink()` on each. `GetWorldInstance` gives direct access to the `WorldInstance` for richer queries (update rate, dirty page count, etc.).

**Game-thread loop (per session, per frame):**
```cpp
session.Tick();
if (const World* world = session.GetWorldView("TestWorld"_n))
{
    enttScene.Gather(*world);   // → RenderScene
}
```

Or via `WorldInstance` directly:
```cpp
session.Tick();
if (WorldInstance* wi = session.GetWorldInstance("TestWorld"_n))
{
    if (const World* world = wi->GetWorldView())
        enttScene.Gather(*world);
}
```

---

## Multi-Session / Multi-World

A process can have many `SessionInstance`s. Each session can have many worlds. The invariants hold at every scale:

- Each `(session, world)` pair has its own `WorldDoubleBuffer` — `EntityId` values are scoped to a world, so they never collide across worlds or sessions.
- `SceneEntitySync` keys its state by `const World*` (the address of the World object), which is unique across all sessions and worlds for the lifetime of the world.
- The game thread calls `session.Tick()` once per session per frame, then reads each world view independently.

---

## What Lives Where

| Concern | Thread | Layer |
|---|---|---|
| Simulation step | Sim | `Phoenix.Sim`, `Phoenix.Runtime` |
| Dirty-page world sync | Sim (inside `OnSimUpdate`) | `WorldDoubleBuffer` |
| Double-buffer swap | Game (`Tick()`) | `WorldDoubleBuffer` |
| Entity spawn/update/despawn | Game | `SceneEntitySync`, `IEntitySyncHandler` |
| Render primitive collection | Game | `ISceneLayer`, `EnTTScene` |
| Draw calls | Render | `IRenderer`, `SDL3Renderer` |
