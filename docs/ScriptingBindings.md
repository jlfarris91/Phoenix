# Scripting Bindings Architecture

This document describes the runtime-agnostic scripting binding system that connects C++ features to the Lua/WASM scripting layer.

---

## Overview

The scripting stack has three layers:

```
┌──────────────────────────────────────────────────────────┐
│  PhoenixLua                                              │
│  FeatureLua : IFeature                                   │
│  LuaWasmEnvironment : WasmEnvironment                    │
│  Loads lua.wasm + .lua scripts; drains EnqueueScript     │
└──────────────────────┬───────────────────────────────────┘
                       │ uses
┌──────────────────────▼───────────────────────────────────┐
│  PhoenixScript                                           │
│  FeatureScript : IFeature                                │
│  WasmRuntime  — shared .wasm bytes + host registrations  │
│  WasmEnvironment — per-world wasm3 runtime               │
│  Fires lifecycle exports: OnWorldUpdate, etc.            │
└──────────────────────┬───────────────────────────────────┘
                       │ uses
┌──────────────────────▼───────────────────────────────────┐
│  PhoenixSim                                              │
│  IScriptBindings  — declarative host-function DDL        │
│  ScriptModuleBuilder — fluent builder for classes/events  │
│  IScriptRuntime   — VM-agnostic registration contract    │
│  TypeRegistry     — auto-discovers IScriptBindings impls │
└──────────────────────────────────────────────────────────┘
```

No module below `PhoenixLua` ever includes Lua or WASM headers. All binding declarations use `IScriptBindings` + `ScriptModuleBuilder`, both in `PhoenixSim`.

---

## `IScriptBindings`

`src/PhoenixSim/Scripting/IScriptBindings.h`

Declarative DDL interface for script bindings that cannot be expressed via `PHX_DEFINE_TYPE` reflection. Implementations are discovered at runtime via `TypeRegistry::GetAllDerivedFrom<IScriptBindings>()`.

```cpp
class IScriptBindings
{
    PHX_DECLARE_TYPE_INTERFACE(IScriptBindings)
public:
    virtual void Describe(ScriptModuleBuilder& builder) const = 0;
};
```

Each implementation is default-constructed, `Describe()` is called, then destroyed. Implementations must be default-constructible.

**Existing implementations:**

| Class | Module | What it registers |
|---|---|---|
| `SimScriptBindings` | PhoenixSim | `Phoenix.World`, `Phoenix.Random`, `Phoenix.Entity`, Blackboard |
| `RTSScriptBindings` | PhoenixRTS | `Phoenix.Unit` class with instance methods |
| `SteeringScriptBindings` | PhoenixSteering | Steering/movement API |

---

## `ScriptModuleBuilder`

`src/PhoenixSim/Scripting/ScriptModuleBuilder.h`

Fluent builder used inside `IScriptBindings::Describe`. Supports three kinds of declarations:

### Namespace functions

```cpp
builder.Namespace("Phoenix.World")
    .Function("GetId(world)",   &PhxSim_World_GetId)
    .Function("GetType(world)", &PhxSim_World_GetType);
```

The declaration string `"GetId(world)"` is validated at compile time against the actual function signature via `MethodDeclarationString<TArgs...>`. Mismatches are a compile error.

World-typed parameters are automatically injected by the WASM trampoline — Lua callers do not pass them.

### OOP class metatables

```cpp
builder.Class<UnitId>("Phoenix.Unit")
    .Inherits("Phoenix.Entity")
    .Method("IsAlive(world, unit)",           &FeatureUnit::UnitIsAlive)
    .Method("IssueCommand(world, unit, cmd)", &FeatureOrders::StaticIssueCommand);
```

`LuaGen` composes these into Lua metatables so Lua scripts can call `unit:IsAlive()` as instance methods. `WasmGen` flattens them to static host imports.

### Template methods (one variant per type)

```cpp
builder.Namespace("Phoenix.World")
    .TEMPLATE_FUNCTION("GetBlackboardValue(world, key, default)",
                       &PhxSim_GetBlackboardValue, BlackboardTypes);
```

Registers one function per type in `BlackboardTypes`, named `GetBlackboardValue_int32`, `GetBlackboardValue_FName`, etc.

### Script events (delegate → WASM export)

```cpp
builder.Namespace("Phoenix.Unit")
    .Event("OnUnitSpawned(unit)", &FeatureUnit::GetOnUnitSpawnedDelegate);
```

When the delegate fires, `WasmEnvironment` calls the WASM export of the same name. The Lua script just defines a global:

```lua
function OnUnitSpawned(unit)
    -- unit is a UnitId
end
```

Subscriptions are set up in `FeatureScript::OnWorldInitialize` and removed in `WasmEnvironment`'s destructor.

---

## `WasmRuntime` and `WasmEnvironment`

### `WasmRuntime` (`src/PhoenixScript/WasmRuntime.h`)

Shared per `.wasm` file. On `LoadFile`:
1. Reads raw WASM bytes from disk.
2. Iterates `TypeRegistry::GetAll()` collecting static methods as host imports.
3. Instantiates every `IScriptBindings`, calls `Describe`, collects functions/classes/events.

Owned by `FeatureScript`. Multiple worlds share the same `WasmRuntime`.

### `WasmEnvironment` (`src/PhoenixScript/WasmEnvironment.h`)

One per world. Owns an independent `wasm3` `IM3Runtime` (linear memory). Worlds can step in parallel with no shared scripting state.

Key operations:
- `CallExport(name, argc, argPtrs, ...)` — calls a named WASM export.
- `SubscribeToEvents(world, events)` — subscribes delegate handlers for all registered script events.
- `Snapshot()` / `Restore()` — copies linear memory for rollback.

**World-parameter injection:** Any host function parameter typed as `WorldRef` or `WorldConstRef` is detected by `TypeDescriptor` and injected automatically by the trampoline. Lua callers never pass a world argument.

### `LuaWasmEnvironment` (`src/PhoenixLua/LuaWasmEnvironment.h`)

Subclass of `WasmEnvironment` that adds Lua-specific operations:
- `LoadLuaScript(path)` — writes .lua file bytes into the WASM script buffer via `GetScriptBuffer`/`LoadScript` exports.
- `RunString(code)` — executes a Lua snippet inside the running Lua state.

---

## Lifecycle callbacks

`FeatureScript` calls these named WASM exports automatically. All are optional — missing exports are silently skipped.

| Export | Arguments | When called |
|---|---|---|
| `OnWorldInitialize` | — | World is initialized |
| `OnWorldShutdown` | — | World is shutting down |
| `OnPreWorldUpdate` | `float dt` | Before world update |
| `OnWorldUpdate` | `float dt` | During world update |
| `OnPostWorldUpdate` | `float dt` | After world update |
| `OnPreUpdate` | — | Session pre-update |
| `OnUpdate` | — | Session update |
| `OnPostUpdate` | — | Session post-update |

Script events registered via `IScriptBindings::Event(...)` add additional exports on top of these.

---

## Adding bindings for a new module

1. Create `MyScriptBindings : public IScriptBindings` in the module.
2. Use `PHX_DEFINE_TYPE` to register it with the TypeRegistry.
3. Implement `Describe(ScriptModuleBuilder&)` using `Namespace`, `Class`, `Event`, etc.
4. No wiring needed — `TypeRegistry::GetAllDerivedFrom<IScriptBindings>()` discovers it automatically.

**For script events**, the feature needs a static delegate accessor:

```cpp
// In the feature header:
PHX_DECLARE_MULTICAST_DELEGATE(FOnUnitSpawned, UnitId);
static FOnUnitSpawned& GetOnUnitSpawnedDelegate(WorldRef world);

// In IScriptBindings::Describe:
builder.Namespace("Phoenix.Unit")
    .Event("OnUnitSpawned(unit)", &FeatureUnit::GetOnUnitSpawnedDelegate);
```

---

## Code generation tools

| Tool | Output | Purpose |
|---|---|---|
| `PhoenixAPIGen` | `api.json` | Dumps full type + binding API as JSON |
| `PhoenixWasmGen` | `host_api.h` | C header declaring WASM host imports for Emscripten |
| `PhoenixLuaGen` | `lua_bridge.c` | Lua C glue compiled into `lua.wasm` |

These run at native build time only (`if(NOT EMSCRIPTEN)` in CMake). The `.wasm` outputs are checked in or distributed alongside the application.

---

## File map

| File | Module | Purpose |
|---|---|---|
| `src/PhoenixSim/Scripting/IScriptBindings.h` | PhoenixSim | Binding declaration interface |
| `src/PhoenixSim/Scripting/IScriptRuntime.h` | PhoenixSim | VM-agnostic runtime contract |
| `src/PhoenixSim/Scripting/ScriptModuleBuilder.h` | PhoenixSim | Fluent builder: Namespace/Class/Event |
| `src/PhoenixSim/Scripting/SimScriptBindings.h/cpp` | PhoenixSim | World/Random/Blackboard/Entity bindings |
| `src/PhoenixRTS/Scripting/RTSScriptBindings.h/cpp` | PhoenixRTS | Unit/Orders bindings |
| `src/PhoenixScript/WasmRuntime.h/cpp` | PhoenixScript | Shared WASM bytes + host registrations |
| `src/PhoenixScript/WasmEnvironment.h/cpp` | PhoenixScript | Per-world wasm3 runtime |
| `src/PhoenixScript/FeatureScript.h/cpp` | PhoenixScript | Lifecycle dispatch to WASM exports |
| `src/PhoenixLua/LuaWasmEnvironment.h/cpp` | PhoenixLua | Lua script loading on top of WasmEnvironment |
| `src/PhoenixLua/FeatureLua.h/cpp` | PhoenixLua | Lua feature: registers world runtime, drains script queue |
