# WASM Scripting Internals

Reference for `PhoenixScript` / `PhoenixLua` / `WasmEnvironment` — the wasm3-based Lua scripting layer.

---

## Architecture

```
FeatureLua  (PhoenixLua)
  │  reads "script" key from world config → .lua path
  │  calls FeatureScript::RegisterWorldRuntime(world, "Data/LuaRunner.wasm")
  │  calls WasmEnvironment::LoadLuaScript(luaPath)
  │  drains EnqueueScript queue via WasmEnvironment::RunString
  │
  ▼
FeatureScript  (PhoenixScript)
  │  one WasmRuntime per unique .wasm file (shared across worlds)
  │  one WasmEnvironment per world (independent linear memory)
  │  drives all WASM lifecycle callbacks
  │
  └─► WasmRuntime      — loads raw .wasm bytes, owns IM3Environment
  └─► WasmEnvironment  — owns IM3Runtime (linear memory), links imports, calls exports
```

**Binary split**: `LuaRunner.wasm` is a generic Lua 5.4 interpreter — it knows nothing about
any specific script. It lives at `Data/LuaRunner.wasm` and is shared across all worlds in a
session. World-specific Lua source lives in `Data/Worlds/<world>/script.lua` and is pushed
into WASM linear memory at runtime via `LoadLuaScript`.

**Feature registration order matters**: `FeatureLua` must be registered before `FeatureScript`
in the service container. `FeatureLua::OnWorldInitialize` pre-creates the environment (via
`RegisterWorldRuntime`) and loads the Lua script. `FeatureScript::OnWorldInitialize` then finds
the already-created environment and fires `CallVoid("OnWorldInitialize")` on it.

**Build pipeline** (CMake, `src/PhoenixLuaWasm/CMakeLists.txt`):
1. `PhoenixWasmGen` → `host_api.h` + `lua_bridge.c` (host function stubs from TypeRegistry)
2. `emcc(lua_wasm.c + lua_bridge.c + Lua 5.4 sources)` → `LuaRunner.wasm`

---

## FeatureLua ↔ FeatureScript Split

`FeatureScript` is the generic WASM host — it has **no Lua-specific logic**. It exposes:

```cpp
// Pre-create a WasmEnvironment for a world from an external WASM path.
// Returns the env so the caller can configure it (e.g. LoadLuaScript).
// FeatureScript::OnWorldInitialize will find it and fire lifecycle callbacks.
WasmEnvironment* RegisterWorldRuntime(WorldRef world, const std::filesystem::path& wasmPath);

// Returns the environment for a world, or nullptr.
WasmEnvironment* GetEnvironment(WorldRef world) const;
```

`FeatureLua` is the Lua orchestration layer — it knows about `LuaRunner.wasm`, `.lua` files,
and `EnqueueScript`. Its `OnWorldInitialize`:

```cpp
// 1. Read lua script path from world's FeatureLua config ("script" key)
// 2. Resolve LuaRunner.wasm: Session->GetDataDirectory() / "LuaRunner.wasm"
// 3. featureScript->RegisterWorldRuntime(world, runnerPath)  → WasmEnvironment*
// 4. env->LoadLuaScript(luaPath)
// FeatureScript fires OnWorldInitialize WASM export after this returns
```

`FeatureLua::OnWorldUpdate` drains the `EnqueueScript` queue:
```cpp
// Gets the env from featureScript->GetEnvironment(world)
// Calls env->RunString(code) for each pending snippet
```

**World config** (`DefaultWorld.json`):
```json
"FeatureLua": { "script": "./DefaultWorld/script.lua" }
```
No `FeatureScript` entry needed when using `FeatureLua` — it manages the WASM env itself.

---

## wasm3 Raw-Function Stack Convention

This is the most non-obvious thing in the whole system and was the root cause of the
ACCESS_VIOLATION during initial bringup.

In a wasm3 raw host function (`M3RawCall` signature: `const void* _fn, IM3Runtime, uint64_t* _sp, void* _mem`):

- **`sp[0 .. numReturns-1]`** are the **return slots** (write-only from the host's perspective)
- **`sp[numReturns ..]`** are the **arguments** (read-only from the host's perspective)

The `m3ApiReturnType(TYPE*)` macro INCREMENTS `sp` by one slot, so after calling it the
local `sp` variable already points at the first argument. This is why handwritten raw functions
that call `m3ApiReturnType` read `sp[0]` as the first arg — they already advanced past the
return slot.

**Concrete examples:**

```
// fd_write: signature i(iiii)  → 1 return, 4 args
// Before m3ApiReturnType: sp[0]=return, sp[1]=fd, sp[2]=iovs_ptr, sp[3]=iovs_len, sp[4]=nwritten_ptr
// After  m3ApiReturnType: sp advanced, local sp[0]=fd, sp[1]=iovs_ptr ...

// WasmHostTrampoline for a function returning void (0 returns):
//   argSp = sp + 0  (args start at sp[0])
// WasmHostTrampoline for a function returning i32 (1 return):
//   argSp = sp + 1  (args start at sp[1], sp[0] is the return slot)
```

In `WasmEnvironment.cpp`, the trampoline uses:
```cpp
const bool hasReturn = !fn.Return.Type.IsVoid();
uint64_t* argSp = hasReturn ? (sp + 1) : sp;
```

---

## Emscripten STANDALONE_WASM Conventions

Built with `-sSTANDALONE_WASM=1` (pure `.wasm`, no JS glue). Key implications:

1. **No WASM start section** — there is no auto-called init. Instead Emscripten exports `_initialize`
   which calls `__wasm_call_ctors` (C++ static initializers, Lua global state setup).
   **Must call `_initialize` explicitly** after all imports are linked, before any other export.

2. **`invoke_*` imports for setjmp/longjmp** — Emscripten wraps `call_indirect` through host
   imports named like `invoke_v`, `invoke_vii`, `invoke_iii`, etc. wasm3 can't resolve these
   automatically; `WasmEnvironment` registers a `DispatchTableCall` handler for them.

   Stack convention for `invoke_*`:
   - **void return** (`invoke_v`, `invoke_vi`, …): `sp[0]` = function table index, `sp[1..]` = target args
   - **non-void return** (`invoke_i`, `invoke_ii`, …): `sp[0]` = return slot, `sp[1]` = table index, `sp[2..]` = target args

3. **Memory is fixed-size** — compiled with `-sALLOW_MEMORY_GROWTH=0`, `INITIAL_MEMORY=MAXIMUM_MEMORY=16MiB`.
   This is intentional: enables `Snapshot()`/`Restore()` via a single `memcpy` of the full linear memory.

---

## Lua Script Lifecycle

The WASM binary exports these functions for Lua script management (all use the 128KB `g_script_buf`):

| Export | Signature | Purpose |
|---|---|---|
| `GetScriptBuffer` | `() → i32` | Returns WASM linear-memory address of the shared script buffer |
| `LoadScript` | `(i32 len) → i32` | Loads new script — closes any live `lua_State` so next lifecycle call re-initializes |
| `RunString` | `(i32 len) → i32` | Executes a Lua snippet in the **existing** `lua_State`; global state is preserved |

**`LoadLuaScript`** (host side, `WasmEnvironment`):
1. Read `.lua` file bytes
2. `GetScriptBuffer()` → `wasmPtr`
3. `memcpy(mem + wasmPtr, bytes, len)`
4. `LoadScript(len)` — tears down old `lua_State`
5. Next lifecycle callback calls `ensure_state()` which re-creates the state and runs the script body

**`RunString`** (host side, `WasmEnvironment::RunString`):
1. `GetScriptBuffer()` → `wasmPtr`
2. `memcpy(mem + wasmPtr, code, len)`
3. `RunString(len)` — runs `luaL_loadbuffer` + `lua_pcall` on existing state
4. Used by `FeatureLua::OnWorldUpdate` to drain `EnqueueScript` queue

`ReloadLuaScript()` just calls `LoadLuaScript(LuaScriptPath)` — the state is torn down and
rebuilt on the next callback, picking up any changes to the `.lua` file on disk.

---

## Calling WASM Exports

Use the higher-level `m3_Call` / `m3_GetResults` API (not raw `m3_CallV` for anything with args).

**No-arg void exports** (lifecycle callbacks): use `m3_CallV(fn)`.

**Exports with args/returns** (GetScriptBuffer, LoadScript, RunString):
```cpp
// i32 GetScriptBuffer()
m3_Call(getBuf, 0, nullptr);
uint32_t wasmPtr = 0;
const void* retPtrs[] = { &wasmPtr };
m3_GetResults(getBuf, 1, retPtrs);

// i32 LoadScript(i32 len)  /  RunString(i32 len)
const int32_t len = static_cast<int32_t>(bytes.size());
const void* argPtrs[] = { &len };
m3_Call(fn, 1, argPtrs);
int32_t result = -1;
const void* resPtr[] = { &result };
m3_GetResults(fn, 1, resPtr);
```

---

## WASM Signature Generation

`PhoenixWasmGen` scans the `TypeRegistry` at startup and emits `host_api.h` + `lua_bridge.c`. The
host-side `WasmEnvironment` does the same computation at runtime to build the signature string it
passes to `m3_LinkRawFunctionEx`. These two must agree character-for-character or wasm3 will
crash inside `m3_core.c:556` (`d_m3Assert(pcFound)`).

### How `BuildWasmSignature` works

```
BuildWasmSignature(method)
  → AppendReturn:   AppendTypeChar(returnTypeRef)
  → AppendArgs:     for each param: AppendTypeChar(paramTypeRef)

AppendTypeChar(typeRef)
  if IsExpandableStruct (IsStruct() && desc->GetSize() <= 32)
    → AppendStructFieldChars(desc)   // recurse into struct fields alphabetically
  else
    → single char: i/I/f/F/…

AppendStructFieldChars(desc)
  → for each property (alphabetical):
      AppendTypeChar(prop->GetTypeRef())   // NOT MakeGenericValueTypeRef!
```

### TFixed as scalar vs struct

`TFixed<Tb,T>` types (Distance, Angle, Speed, Time) are registered in `TypeRegistry` with
`REGISTER_FIXED_TYPE` — they have no fields, just `FractionalBits` metadata. They must be
treated as scalar `i`/`I` chars, not expanded as (empty) structs.

The fix is two-pronged:

1. **`GenericValueTypeRefMaker<TFixed<Tb,T>>`** specialization in `Reflection.h` — returns
   `{FixedPoint, &TypeRegistry::GetOrCreate<TFixed>()}`. This makes `MakeGenericValueTypeRef`
   emit `FixedPoint` (non-struct), which `AppendTypeChar` maps to a single `i` char.

2. **`IsExpandableStruct`** already excludes zero-field structs: TFixed descriptors have no
   field properties, so even before the fix they produced 0 chars — the real symptom.

### Dead-stripping of `FixedTypeRegistrations.obj`

`PhoenixWasmGen` links with `/WHOLEARCHIVE:PhoenixSim` — every `.obj` is included regardless of
symbol references. `TestRTS` does **not** use `/WHOLEARCHIVE`, so MSVC only includes `.obj` files
that define symbols referenced by other TUs.

`FixedTypeRegistrations.cpp` contains only `static const bool` lambdas — no external symbols.
MSVC silently drops it, leaving Vec2 and Transform2D with **no field registrations** at runtime.
`AppendStructFieldChars` iterates 0 fields → Vec2 produces 0 chars → `TransformComponent` param
in any method signature is empty → wrong WASM signature.

**Fix**: `FixedTypeRegistrations.cpp` exports an external no-op:

```cpp
void Phoenix_EnsureFixedTypeRegistrations() {}   // in FixedTypeRegistrations.cpp
```

`TransformComponent.cpp` calls it at static-init time:

```cpp
extern void Phoenix_EnsureFixedTypeRegistrations();
static const bool s_FixedTypesLoaded = (Phoenix_EnsureFixedTypeRegistrations(), true);
```

This forces the linker to include `FixedTypeRegistrations.obj` without `/WHOLEARCHIVE`.

### `PHX_WASM_VERBOSE` diagnostic

`#define PHX_WASM_VERBOSE 1` in `WasmEnvironment.cpp` logs every `m3_LinkRawFunctionEx` call:

```
[Info]: [WASM] Linked: SpawnUnit  i(iiiii)
[Info]: [WASM] Linked: get_ZCode  I(iiiiiI)
[Info]: [WASM] Link mismatch: get_ZCode  sig=I(iI)  host=I(iiiiiI)   ← wrong!
```

The "Link mismatch" line (always emitted on error) shows both the WASM-side signature and what
the host computed — useful for diagnosing signature mismatches. Turn off (`#define … 0`) once
all functions link correctly.

---

## Diagnostics

**wasm3 strace** — set at compile time, extremely verbose:
```cmake
target_compile_definitions(m3 PUBLIC d_m3EnableStrace=2)
```
Traces every WASM function call (name, args, result) to stderr. Disable for normal builds.

**Backtraces** — lower overhead:
```cmake
target_compile_definitions(m3 PUBLIC d_m3RecordBacktraces=1)
```
Enables `m3_GetBacktrace()` after a trap. Currently left on in debug builds.

**Log markers in `ensure_state()`** — `lua_wasm.c` prints `[lua_wasm] <step>` for each
init phase. In Phoenix these appear as `[Info]: [Lua] [lua_wasm] ...` because `printf` in
the WASM routes through `fd_write` → the host's log sink.

---

## Key Files

| File | Purpose |
|---|---|
| `src/PhoenixLua/FeatureLua.cpp/.h` | Lua orchestration: RegisterWorldRuntime, LoadLuaScript, EnqueueScript drain |
| `src/PhoenixScript/FeatureScript.cpp/.h` | Generic WASM host: RegisterWorldRuntime API, per-world lifecycle dispatch |
| `src/PhoenixScript/WasmEnvironment.cpp/.h` | wasm3 runtime, import linking, export dispatch, LoadLuaScript, RunString |
| `src/PhoenixScript/WasmRuntime.cpp/.h` | Loads raw .wasm bytes, owns IM3Environment |
| `src/PhoenixLuaWasm/lua_wasm.c` | Lua interpreter WASM entry points: GetScriptBuffer, LoadScript, RunString, lifecycle exports |
| `src/PhoenixLuaWasm/CMakeLists.txt` | Emscripten build pipeline + `add_lua_wasm_script()` function |
| `tools/PhoenixWasmGen/PhoenixWasmGen.cpp` | Generates `host_api.h` + `lua_bridge.c` from TypeRegistry |
| `tests/TestRTS/Data/LuaRunner.wasm` | Built artifact — generic Lua 5.4 interpreter, shared across worlds |

---

## Common Mistakes

| Mistake | Symptom | Fix |
|---|---|---|
| Wrong sp offsets in raw host function | ACCESS_VIOLATION or corrupt state | Account for return slots: args start at `sp[numReturns]` |
| Forgetting `_initialize` call | Lua global vars not initialized, crashes in `luaL_newstate` | Call `_initialize` after linking all imports, before any other export |
| Wrong `invoke_*` table index offset | "table index out of range" trap | void invokes: idx=sp[0]; non-void invokes: idx=sp[1] |
| `m3_CallV` on export with args | Silently passes garbage | Use `m3_Call` + `m3_GetResults` for exports with parameters |
| `add_compile_definitions` for m3 flags | Flags silently ignored | Use `target_compile_definitions(m3 PUBLIC ...)` after `FetchContent_MakeAvailable` |
| `FeatureScript` registered before `FeatureLua` | `RegisterWorldRuntime` not called before `OnWorldInitialize` — env missing | Register `FeatureLua` before `FeatureScript` in the service container |
| `FeatureScript` world config present alongside `FeatureLua` config | Two environments created for same world | When using `FeatureLua`, omit the `FeatureScript` world config entry |
| `FixedTypeRegistrations.obj` dead-stripped from non-WHOLEARCHIVE link | Vec2/Transform2D have no fields → 0 chars in WASM signature → crash at `m3_core.c:556` | `Phoenix_EnsureFixedTypeRegistrations()` force-include in `TransformComponent.cpp` |
| Lua script passes wrong arg count to a host function | `_emscripten_throw_longjmp` → wasm3 `GetBacktraceOffset` → `d_m3Assert(pcFound)` crash | Fix the Lua call site to match the current `host_api.h` signature; check `lua_bridge.c` for expected param count |
