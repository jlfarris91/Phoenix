# Scripting Bindings Architecture

This document specifies the design for a runtime-agnostic scripting binding system across the PhoenixSim module stack. It covers the two core interfaces (`IScriptRuntime` and `IScriptBindings`), how they fit the dependency graph, how the reflection system is leveraged for automatic type exposure, how bindings are discovered at session init, and how the `Action` verb system is made scriptable.

---

## 1. Problem Statement

`FeatureLua.cpp` currently hardcodes bindings for `FeatureUnit`, `FeatureOrders`, `FeatureVitals`, and `FeaturePhysics` with direct sol2 API calls. This creates three problems:

1. **Wrong dependency direction.** `PhoenixLua` depends on `PhoenixRTS` and `PhoenixPhysics` at link time. The scripting bridge should sit _above_ modules, not require them.
2. **No extension point.** Adding any new bindable module requires editing `FeatureLua.cpp`.
3. **Single runtime.** sol2 is the only possible runtime. Replacing or augmenting it (QuickJS, Wren) requires a full rewrite of the binding layer.

---

## 2. Dependency Diagram

```
┌──────────────────────────────────────────────────┐
│  PhoenixLua  (or PhoenixQuickJS, PhoenixWren …)  │
│                                                  │
│  FeatureLua : IFeature                           │
│  LuaRuntime : IScriptRuntime                     │
│  Links: sol2, lua                                │
└────────────────────┬─────────────────────────────┘
                     │ depends on
       ┌─────────────┼────────────────┐
       ▼             ▼                ▼
 ┌──────────┐  ┌───────────┐  ┌────────────────┐
 │PhoenixRTS│  │PhoenixPhy-│  │PhoenixSteering │
 │          │  │sics       │  │                │
 │ RTSScript│  │PhysicsScr-│  │SteeringScript  │
 │ Bindings │  │iptBindings│  │Bindings        │
 │:IScript  │  │:IScript   │  │:IScript        │
 │ Bindings │  │Bindings   │  │Bindings        │
 └────┬─────┘  └─────┬─────┘  └───────┬────────┘
      │               │                │
      └───────────────┼────────────────┘
                      │ depends on
                      ▼
       ┌──────────────────────────────────┐
       │           PhoenixSim             │
       │                                  │
       │  IScriptRuntime   (NEW)          │
       │  IScriptBindings  (NEW)          │
       │  ScriptBindingRegistry (NEW)     │
       │                                  │
       │  IFeature, IService              │
       │  TypeDescriptor, TypeRegistry    │
       │  Action, FName                   │
       └──────────────────────────────────┘
```

No module below `PhoenixLua` ever includes a scripting library header. The only shared headers are `IScriptRuntime.h` and `IScriptBindings.h`, both in `PhoenixSim`.

---

## 3. Interface: `IScriptRuntime`

Lives in `src/PhoenixSim/Scripting/IScriptRuntime.h`. All methods are in terms of PhoenixSim primitives — no sol2, no lua.h.

```cpp
class IScriptRuntime
{
public:
    // ── Type registration ─────────────────────────────────────────────────────
    // Walks TypeDescriptor::Properties and generates getter/setter accessors
    // for each reflected field. The runtime maps EPropertyValueType to
    // script-native types (Int32 → number, Bool → boolean, FName → integer).
    virtual void RegisterType(const TypeDescriptor& descriptor) = 0;

    // ── Namespace management ──────────────────────────────────────────────────
    // Opens a dot-separated namespace/table (e.g. "Phoenix.Unit").
    // Subsequent RegisterFunction calls are placed inside it.
    virtual void OpenNamespace(const char* dotSeparatedPath) = 0;
    virtual void CloseNamespace() = 0;

    // ── Function registration ─────────────────────────────────────────────────
    virtual void RegisterFunction(FName name, ScriptFunctionWrapper fn) = 0;
    virtual void RegisterIntConstant(FName name, int64 value) = 0;
    virtual void RegisterStringConstant(FName name, const char* value) = 0;

    // ── Callbacks (script → C++ events) ──────────────────────────────────────
    // Scripts write a global function with this name; the C++ side calls
    // InvokeCallback to fire it.
    virtual void RegisterCallback(FName name) = 0;
    virtual bool InvokeCallback(FName name, const ScriptCallArgs& args) = 0;

    // ── Script loading ────────────────────────────────────────────────────────
    virtual bool LoadFile(const char* path) = 0;
    virtual bool ExecString(const std::string& code) = 0;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    // Called once after all bindings have been registered, before any script
    // is loaded.
    virtual void OnBindingsComplete() = 0;
};
```

### `ScriptFunctionWrapper` and `ScriptCallArgs`

Thin type-erased wrappers defined in `IScriptRuntime.h`. They carry arguments and return values as tagged `TypedData` unions, reusing the existing `EDataType` / `Data` union from `Actions.h`. No heap allocation in the common case.

The runtime adapter (`LuaRuntime`) translates between these wrappers and the sol2/Lua stack internally. Modules never see sol2.

---

## 4. Interface: `IScriptBindings`

Lives in `src/PhoenixSim/Scripting/IScriptBindings.h`.

### Design choice: separate companion class, not on `IFeature`

`IScriptBindings` is a **separate interface**, not added to `IFeature` or `IService` directly. Reasons:

- Not every feature needs scripting — inheriting it unconditionally bloats all features.
- A single class can cover multiple features (e.g. `RTSScriptBindings` covers Unit + Orders + Vitals).
- Keeps `IFeature` from growing further.
- Allows a module to ship exactly one bindings class discovered via the service container.

Since `IService` is the base for all registered services, `IScriptBindings` extends `IService`. The runtime discovers all `IScriptBindings` instances from the `ServiceContainer`.

```cpp
class IScriptBindings : public IService
{
public:
    // Dot-separated namespace this class owns (e.g. "Phoenix.Unit").
    // The runtime opens this namespace before calling Register and closes it after.
    virtual const char* GetNamespace() const = 0;

    // Called once at session init, after the runtime is constructed but before
    // any script is loaded. Implementations call runtime.RegisterFunction, etc.
    virtual void Register(IScriptRuntime& runtime, Session& session) = 0;
};
```

---

## 5. Reflection Integration

### `RegisterType` auto-exposes reflected fields

When a bindings class calls `runtime.RegisterType(SomeComponent::GetStaticTypeDescriptor())`, the runtime walks `TypeDescriptor::Properties` and generates getter/setter accessors for each `PropertyDescriptor`:

```
for each PropertyDescriptor& prop in descriptor.Properties:
    scriptType.addProperty(
        name  = prop.Name,
        get   = [&prop](const void* obj) -> ScriptValue { /* dispatch on prop.ValueType */ },
        set   = [&prop](void* obj, ScriptValue v)       { /* unpack by prop.ValueType */  }
    )
```

`EPropertyValueType` already covers all primitives the script layer needs: `Int32`, `Float`, `Bool`, `String`, `Name`, `Vec2`, `FixedPoint`, `Transform2D`. The runtime adapter implements the type switch once; all registered types get property binding for free.

### World-context properties

Properties backed by `StaticWorldPropertyAccessor` require a `WorldRef`. These are registered as **functions on the namespace**, not as fields on the type:

```lua
-- world-context: function call with explicit worldId
local owner = Phoenix.Unit.GetOwner(worldId, entityId)

-- non-world-context component field: accessed on the object directly (future)
-- local hp = healthComp.Health.Current
```

### Auto-exposure is opt-in

A bindings class explicitly calls `runtime.RegisterType(...)` for each type it wants to expose. Types that are only annotated in `TypeRegistry` are not automatically pushed to scripts. This prevents internal allocator types from leaking into the script namespace.

---

## 6. File Locations

| Header | Module | Purpose |
|---|---|---|
| `src/PhoenixSim/Scripting/IScriptRuntime.h` | PhoenixSim | VM-agnostic registration contract |
| `src/PhoenixSim/Scripting/IScriptBindings.h` | PhoenixSim | Per-module binding contract |
| `src/PhoenixSim/Scripting/ScriptBindingRegistry.h/cpp` | PhoenixSim | Optional static self-registration |
| `src/PhoenixLua/LuaRuntime.h/cpp` | PhoenixLua | sol2 adapter implementing `IScriptRuntime` |
| `src/PhoenixRTS/Scripting/RTSScriptBindings.h/cpp` | PhoenixRTS | Unit, Orders, Vitals bindings |
| `src/PhoenixPhysics/Scripting/PhysicsScriptBindings.h/cpp` | PhoenixPhysics | Physics bindings |
| `src/PhoenixSteering/Scripting/SteeringScriptBindings.h/cpp` | PhoenixSteering | Steering/pathfinding bindings |

`FeatureLua.h/cpp` are **modified, not replaced**. `FeatureLua` retains its role as the `IFeature` that owns the lifecycle (session block with `sol::state`, `EnqueueScript`, action callbacks) — it delegates binding registration to `LuaRuntime` + `IScriptBindings` rather than doing it inline.

---

## 7. Registration Flow

### Session construction (app side)

```cpp
ServiceContainerBuilder builder;

// Core features (unchanged)
builder.RegisterService<FeatureUnit>().AsInterfaces();
builder.RegisterService<FeatureOrders>().AsInterfaces();
builder.RegisterService<FeaturePhysics>().AsInterfaces();

// Scripting bindings — each registered as IScriptBindings
builder.RegisterService<RTSScriptBindings>().As<IScriptBindings>();
builder.RegisterService<PhysicsScriptBindings>().As<IScriptBindings>();
builder.RegisterService<SteeringScriptBindings>().As<IScriptBindings>();

// The Lua feature
auto lua = std::make_shared<FeatureLua>();
lua->SetScriptPath("./Data/script.lua");
builder.RegisterService(lua).AsInterfaces();
```

### `FeatureLua::Initialize` (revised)

```
1. Construct LuaRuntime (owns sol::state, configures base libraries + FP64)
2. Collect all IScriptBindings from session->GetServices<IScriptBindings>()
3. For each IScriptBindings* binding:
       runtime.OpenNamespace(binding->GetNamespace())
       binding->Register(runtime, *session)
       runtime.CloseNamespace()
4. Register built-in callbacks: OnWorldUpdate, OnWorldInitialize, etc.
5. runtime.OnBindingsComplete()
6. runtime.LoadFile(m_scriptPath)   // if path non-empty
```

`FeatureLua` no longer includes any module-specific header. It calls `IScriptBindings::Register` polymorphically.

### `ScriptBindingRegistry` (optional static self-registration)

Alternatively, a static self-registration pattern mirrors `TypeRegistry`: each `IScriptBindings` subclass calls `ScriptBindingRegistry::Register()` at static-init time. `FeatureLua::Initialize` checks both sources (ServiceContainer + ScriptBindingRegistry) when collecting bindings.

---

## 8. Namespace and Table Structure

```
Phoenix.ECS        — AcquireEntity, ReleaseEntity
Phoenix.Unit       — SpawnUnit, IsAlive, GetOwner, GetUnitData
Phoenix.Orders     — IssueCommand, HasOrders, GetHeadOrderTarget
Phoenix.Vitals     — ApplyDamage, GetHealth, GetMaxHealth
Phoenix.Physics    — ApplyForce, AddExplosionForce, QueryInRange
Phoenix.Steering   — SetMoveTarget, QueryUnitsInRange
Phoenix.GetSimTime()
Phoenix.EnqueueAction(verb, ...)
Phoenix.EnqueueWorldAction(worldId, verb, ...)
```

The top-level `Phoenix` table is created by `FeatureLua` before iterating bindings. Each `IScriptBindings::GetNamespace()` returns the full dotted path including `Phoenix.`. This structure matches the current `FeatureLua.cpp` layout exactly — existing scripts require no changes after the refactor.

---

## 9. Action Scripting

### Dispatching actions from script

`FeatureLua` registers two built-ins (not via `IScriptBindings` — these belong to the runtime feature):

```lua
Phoenix.EnqueueAction(verb, arg0, arg1, ...)
Phoenix.EnqueueWorldAction(worldId, verb, arg0, ...)
```

These map directly to `Session::EnqueueAction`. Arguments are passed as `FName` (integer hash) or numeric types; the receiving `OnHandleWorldAction` handler is responsible for interpreting them.

### Handling actions in script

```lua
function OnHandleWorldAction(worldId, verb, arg0)
    if verb == hash("my_custom_action") then
        -- handle
        return true  -- consumed
    end
    return false
end
```

The `hash()` built-in (registered directly on `FeatureLua`, not via `IScriptBindings`) maps a string to an `FName` for comparison against `Action::Verb`.

---

## 10. Example: `RTSScriptBindings` Declaration

```cpp
// src/PhoenixRTS/Scripting/RTSScriptBindings.h
#pragma once

#include "PhoenixSim/Scripting/IScriptBindings.h"
#include "PhoenixRTS/DLLExport.h"

namespace Phoenix::RTS
{
    // Exposes Unit, Orders, and Vitals APIs to any IScriptRuntime.
    // Register via ServiceContainerBuilder as IScriptBindings.
    //
    // No sol2 or Lua headers here.
    class PHOENIX_RTS_API RTSScriptBindings : public IScriptBindings
    {
    public:
        const char* GetNamespace() const override;  // "Phoenix"
        void Register(IScriptRuntime& runtime, Session& session) override;

    private:
        void RegisterUnit(IScriptRuntime& runtime, Session& session);
        void RegisterOrders(IScriptRuntime& runtime, Session& session);
        void RegisterVitals(IScriptRuntime& runtime, Session& session);
    };
}
```

The `Register` implementation opens sub-namespaces for `Phoenix.Unit`, `Phoenix.Orders`, and `Phoenix.Vitals` within the outer `Phoenix` namespace, calls `runtime.RegisterFunction(...)` for each public static API, and calls `runtime.RegisterType(UnitComponent::GetStaticTypeDescriptor())` for reflected component types.

---

## 11. Hot-Reload

The design supports hot-reload without modification:

- `IScriptRuntime::ExecString` injects replacement function definitions at runtime. Scripts that define logic as named globals can be hot-patched by re-executing the changed source.
- `IScriptRuntime::LoadFile` can be called again from `FeatureLua::EnqueueScript` (the existing thread-safe queue is preserved).
- Binding registration happens once at session init; it does not repeat on hot-reload since the C++ API surface doesn't change at runtime.
- A future `IScriptRuntime::Reset()` can tear down and recreate the script state while replaying all `IScriptBindings::Register` calls, enabling full cold reload.

---

## 12. Migration Path from Current `FeatureLua.cpp`

The existing code can be migrated incrementally without breaking the build at any step:

| Step | Change |
|---|---|
| 1 | Add `IScriptRuntime.h` and `IScriptBindings.h` to `PhoenixSim`. No other changes. |
| 2 | Create `LuaRuntime : IScriptRuntime` in `PhoenixLua`, wrapping the existing `sol::state`. |
| 3 | Create `RTSScriptBindings` in `PhoenixRTS`. Move lambdas from `FeatureLua.cpp` into it, replacing sol2 calls with `runtime.RegisterFunction(...)`. |
| 4 | Create `PhysicsScriptBindings` in `PhoenixPhysics` similarly. |
| 5 | Update `FeatureLua::Initialize` to discover and call bindings polymorphically. |
| 6 | Remove `#include "PhoenixRTS/..."` and `#include "PhoenixPhysics/..."` from `FeatureLua.cpp`. |
| 7 | Remove `PhoenixRTS` link dependency from `PhoenixLua/CMakeLists.txt`. This step validates the graph is correct. |

Steps 3 and 4 can be done in parallel. Step 7 is the validation gate.
