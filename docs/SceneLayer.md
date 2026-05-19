# Scene Layer

The scene layer bridges PhoenixSim entities and the renderer. It runs on the game thread, reads from a stable `WorldDoubleBuffer` view, and produces a `RenderScene` for the render thread.

The design is engine-agnostic: `Phoenix.Scene` defines the interfaces; `Phoenix.Scene.EnTT` is the standalone backend. The same interfaces will be implemented in Unreal, Unity, or any other engine.

---

## Project Structure

```
Phoenix.Scene           — engine-agnostic interfaces and sync logic
Phoenix.Scene.EnTT      — EnTT registry backend (standalone / test app only)
```

`Phoenix.Scene` depends on `Phoenix.Runtime` (for `SessionInstance` + `WorldDoubleBuffer`) and `Phoenix.Renderer` (for `RenderScene`). It does not depend on any specific engine.

`Phoenix.Scene.EnTT` depends on `Phoenix.Scene` and `EnTT::EnTT`. It is excluded from Emscripten builds.

---

## Core Types

### EngineEntity

`src/Phoenix.Scene/EngineEntity.h`

An opaque 64-bit handle representing an entity in the engine layer — an `entt::entity`, an `AActor*`, a `GameObject`, etc. The scene layer never looks inside it.

```cpp
struct EngineEntity { uint64_t Id = 0; bool IsValid() const; };
```

### IEntitySyncHandler

`src/Phoenix.Scene/IEntitySyncHandler.h`

Implement one per sim entity kind. The handler creates, updates, and destroys engine entities in response to sim entity lifecycle events.

```cpp
class IEntitySyncHandler
{
public:
    virtual EngineEntity OnSpawn  (WorldConstRef world, ECS::EntityId simEntity) = 0;
    virtual void         OnUpdate (EngineEntity, WorldConstRef world, ECS::EntityId simEntity) = 0;
    virtual void         OnDespawn(EngineEntity) = 0;
};
```

`OnSpawn` returns the new `EngineEntity`. Return an invalid (zero) `EngineEntity` to opt out — the entity won't be tracked.

### SceneEntitySync

`src/Phoenix.Scene/SceneEntitySync.h`

Maintains a `SimEntity → EngineEntity` mapping per world. On each `Sync(world)` call it:
1. Collects all valid sim entities via `FeatureECS::GetEntities`.
2. Calls `OnDespawn` for any engine entities whose sim entity is gone.
3. Calls `OnSpawn` for new sim entities, `OnUpdate` for existing ones.
4. Dispatches to the handler registered for `entity.Kind`.

```cpp
SceneEntitySync sync;
sync.RegisterHandler("Unit"_n,      std::make_shared<UnitSyncHandler>(registry));
sync.RegisterHandler("Projectile"_n, std::make_shared<ProjectileSyncHandler>(registry));

// Each game frame, after session.Tick():
sync.Sync(*world);
```

**Multi-world / multi-session safety:** State is keyed by `const World*`. The same `SceneEntitySync` instance can safely be called with worlds from different sessions in the same frame. Call `RemoveWorld(world)` when a world is destroyed to trigger `OnDespawn` for all its engine entities and release the tracking state.

---

## EnTT Backend

### IEnTTSceneLayer

`src/Phoenix.Scene.EnTT/IEnTTSceneLayer.h`

The EnTT equivalent of `ISceneLayer`. Layers receive the full EnTT registry alongside the world view and accumulate draw calls into the `RenderScene`.

```cpp
class IEnTTSceneLayer
{
public:
    virtual void Gather(const entt::registry&, WorldConstRef, Renderer::RenderScene&) = 0;
};
```

Layers are where engine components become draw calls. A unit layer might query all entities with a `SpriteComponent` and emit `Sprite2DCall`s.

### EnTTEntitySyncHandler

`src/Phoenix.Scene.EnTT/EnTTEntitySyncHandler.h`

Base class for sync handlers that use the EnTT registry. Provides `Pack`/`Unpack` helpers and holds a `Registry` reference.

```cpp
class MyUnitSyncHandler : public EnTTEntitySyncHandler
{
public:
    using EnTTEntitySyncHandler::EnTTEntitySyncHandler;

    EngineEntity OnSpawn(WorldConstRef world, ECS::EntityId simEntity) override
    {
        entt::entity e = Registry.create();
        Registry.emplace<SpriteComponent>(e, LoadSprite("unit.png"));
        return Pack(e);
    }

    void OnUpdate(EngineEntity handle, WorldConstRef world, ECS::EntityId simEntity) override
    {
        entt::entity e = Unpack(handle);
        auto* transform = ECS::FeatureECS::GetComponent<TransformComponent>(world, simEntity);
        Registry.get<SpriteComponent>(e).Position = transform->Position;
    }

    void OnDespawn(EngineEntity handle) override
    {
        Registry.destroy(Unpack(handle));
    }
};
```

### EnTTScene

`src/Phoenix.Scene.EnTT/EnTTScene.h`

Owns the `entt::registry`, a `SceneEntitySync`, a camera, a render target, and a list of layers. `Gather` syncs entities then collects draw calls from all layers.

```cpp
EnTTScene scene;
scene.Camera = { .Position = {0, 0}, .Zoom = 32.f, .Width = 1280, .Height = 720 };
scene.EntitySync.RegisterHandler("Unit"_n, std::make_shared<MyUnitSyncHandler>(scene.GetRegistry()));
scene.Layers.push_back(std::make_shared<UnitLayer>());

// Each game frame:
Renderer::RenderScene rs = scene.Gather(*world);
// Hand rs to render thread.
```

---

## Engine-Agnostic Design

`IEntitySyncHandler` and `ISceneLayer`/`IEnTTSceneLayer` have no engine types in their signatures — only `EngineEntity` (opaque uint64), `WorldConstRef`, `ECS::EntityId`, and `RenderScene`. This means the same handler logic can be ported to Unreal (`AActor*` packed into `EngineEntity.Id`) or Unity (`GameObject` handle) by swapping the backend implementation without changing the interface.

In Unreal:
```cpp
EngineEntity OnSpawn(WorldConstRef world, ECS::EntityId simEntity) override
{
    AActor* actor = SpawnActor(...);
    return { reinterpret_cast<uint64_t>(actor) };
}
```

In Unity (C# interop):
```cpp
EngineEntity OnSpawn(WorldConstRef world, ECS::EntityId simEntity) override
{
    int goId = UnityBridge::SpawnGameObject("Unit");
    return { static_cast<uint64_t>(goId) };
}
```

---

## Full Game-Thread Loop

```cpp
// One EnTTScene per session/world combination (or shared across worlds if handlers are the same).
// One SceneEntitySync per scene.

void GameThreadTick(SessionInstance& session, EnTTScene& scene, FName worldId)
{
    session.Tick();   // advance all WorldDoubleBuffers for this session

    const World* world = session.GetWorldView(worldId);
    if (!world)
        return;       // sim hasn't produced a frame yet

    Renderer::RenderScene rs = scene.Gather(*world);

    // Hand rs to the render thread (queue, swap, etc.)
    renderQueue.Push(std::move(rs));
}
```

See [ThreadingModel.md](ThreadingModel.md) for the full three-thread picture.
