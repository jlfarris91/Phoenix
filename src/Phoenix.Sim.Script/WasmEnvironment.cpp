#include "Phoenix.Sim.Script/WasmEnvironment.h"

#include <algorithm>
#include <fstream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>   // _resetstkoflw
#endif

// wasm3 public API — only included in this translation unit.
#include <wasm3.h>

// wasm3 internal headers — read-only access for invoke_* dispatch.
// The m3 CMake target adds its source/ directory as a public include path,
// so these internal headers are reachable without modifying the wasm3 sources.
#include "m3_env.h"      // M3Module.table0/table0Size, M3Runtime.memory
#include "m3_compile.h"  // CompileFunction()

#include "WasmRuntime.h"
#include "WasmUtility.h"
#include "Phoenix/Logging.h"
#include "Phoenix/Reflection/Variant.h"
#include "Phoenix.Sim/Worlds.h"
#include "Phoenix.Sim/Scripting/IScriptRuntime.h"

using namespace Phoenix;

// Set to 1 to log every host→WASM import link attempt and unresolved imports.
// Leave at 0 for normal builds — signature mismatches are always logged.
#define PHX_WASM_VERBOSE 0

// ── WASI / Emscripten env stubs ───────────────────────────────────────────────
//
// Emscripten's -sSTANDALONE_WASM=1 emits imports for WASI standard functions
// and a few env helpers.  We provide stubs so every import is resolved before
// m3_LoadModule runs (otherwise wasm3 returns "missing imported function" when
// any WASM function that transitively calls one of them is first compiled).
//
// fd_write is forwarded to LogInfo so Lua's print() output is visible.
// All other stubs return WASI_ERRNO_NOSYS (52) or zero.

static const void* WasiStubEnosys(IM3Runtime, M3ImportContext*, uint64_t* sp, void*)
{
    *sp = 52u;  // WASI ENOSYS
    return nullptr;
}

// ── Emscripten invoke_* dispatch ──────────────────────────────────────────────
//
// Emscripten wraps every WASM call_indirect instruction with an invoke_XYZ
// imported host function so that JS-side setjmp/longjmp can intercept
// exceptions.  In wasm3 (a native C interpreter), we just need to execute
// the table entry directly.
//
// Stack layout (void-return invokes, e.g. invoke_vii):
//   sp[0] = tableIndex,  sp[1..] = args for target function
//   → call target with frame = sp+1  (tableIndex slot is skipped)
//
// Stack layout (non-void-return invokes, e.g. invoke_ii):
//   sp[0] = tableIndex AND the return-value slot
//   sp[1..] = args for target function
//   → call target with frame = sp  (target writes result over tableIndex)

static const void* DispatchTableCall(IM3Runtime runtime, M3ImportContext* ctx,
                                     uint64_t* sp, bool voidReturn)
{
    // Early defensive check before any dereference
    if (!ctx || !ctx->function || !ctx->function->module)
    {
        LogError("[invoke] null ctx/function/module");
        return "invoke: null context";
    }

    // wasm3 convention: sp[0..numReturns-1] = return slots, sp[numReturns..] = args.
    // For void invokes (0 returns): sp[0] = tableIndex.
    // For non-void invokes (1 return): sp[0] = return slot, sp[1] = tableIndex.
    const uint32_t idx = voidReturn ? static_cast<uint32_t>(sp[0])
                                    : static_cast<uint32_t>(sp[1]);
    IM3Module module = ctx->function->module;

    if (!module->table0 || idx >= module->table0Size)
    {
        LogError("[invoke] table index {} out of range", idx);
        return m3Err_trapTableIndexOutOfRange;
    }

    IM3Function fn = module->table0[idx];
    if (!fn)
    {
        LogError("[invoke] table[{}] is null", idx);
        return m3Err_trapTableElementIsNull;
    }

    if (!fn->compiled)
    {
        M3Result err = CompileFunction(fn);
        if (err) { LogError("[invoke] CompileFunction: {}", err); return err; }
    }

    // fn->compiled is a pc_t (void* const*).
    // The first word is a pointer to the first IM3Operation.
    // Dispatch directly into wasm3's compiled op chain.
    //
    // wasm3 convention: sp[0..numReturns-1] = return slots, sp[numReturns..] = args.
    //
    // void invoke (0 returns): sp[0]=tableIndex, sp[1..]=target args.
    //   Target is also void (0 returns): its args start at frame[0].
    //   frame = sp+1 → frame[0]=arg1, frame[1]=arg2 ✓
    //
    // non-void invoke (1 return): sp[0]=return slot, sp[1]=tableIndex, sp[2..]=target args.
    //   Target has 1 return at frame[0], args at frame[1..].
    //   Shift sp[2..] → sp[1..] then frame=sp: frame[0]=return, frame[1]=arg1 ✓
    typedef const void* (*M3OpFn)(void* const*, uint64_t*,
                                  M3MemoryHeader*, int64_t, double);
    void* const* pc = fn->compiled;
    uint64_t* frame;
    if (voidReturn)
    {
        // void invoke (0 returns): sp[0]=tableIdx consumed above; target args at sp[1..].
        // Target is also void-returning (0 returns), so its args start at frame[0].
        // frame = sp+1 → frame[0]=sp[1]=arg1, frame[1]=sp[2]=arg2 ✓
        frame = sp + 1;
    }
    else
    {
        // non-void invoke (1 return): sp[0]=return slot, sp[1]=tableIdx consumed above,
        // target args at sp[2..].  Target has 1 return at frame[0], args at frame[1..].
        // Shift sp[2..] → sp[1..] so frame=sp gives frame[0]=return, frame[1]=arg1 ✓
        for (int i = 1; i < 8; ++i) sp[i] = sp[i + 1];
        frame = sp;
    }
    return reinterpret_cast<M3OpFn>(*pc)(pc + 1, frame,
                                          runtime->memory.mallocated, 0, 0.0);
}

// Void-return invoke_* wrappers
#define INVOKE_VOID(NAME) \
    static const void* NAME(IM3Runtime rt, M3ImportContext* ctx, uint64_t* sp, void*) \
    { return DispatchTableCall(rt, ctx, sp, true); }

INVOKE_VOID(Invoke_v)
INVOKE_VOID(Invoke_vi)
INVOKE_VOID(Invoke_vii)
INVOKE_VOID(Invoke_viii)
INVOKE_VOID(Invoke_viiii)
INVOKE_VOID(Invoke_viiiii)
INVOKE_VOID(Invoke_viiiiii)
#undef INVOKE_VOID

// Non-void-return invoke_* wrappers
#define INVOKE_RET(NAME) \
    static const void* NAME(IM3Runtime rt, M3ImportContext* ctx, uint64_t* sp, void*) \
    { return DispatchTableCall(rt, ctx, sp, false); }

INVOKE_RET(Invoke_i)
INVOKE_RET(Invoke_ii)
INVOKE_RET(Invoke_iii)
INVOKE_RET(Invoke_iiii)
#undef INVOKE_RET

// fd_write(fd, iovs_ptr, iovs_len, nwritten_ptr) -> errno
// Routes all fd writes to LogInfo so Lua print() output is visible.
static const void* WasiFdWrite(IM3Runtime runtime, M3ImportContext*, uint64_t* sp, void*)
{
    // fd_write(fd, iovs_ptr, iovs_len, nwritten_ptr) -> i32
    // sp[0]=fd (return slot), sp[1]=iovs_ptr, sp[2]=iovs_len, sp[3]=nwritten_ptr
    // sp[0] = return slot (i32), sp[1]=fd, sp[2]=iovs_ptr, sp[3]=iovs_len, sp[4]=nwritten_ptr
    const uint32_t fd           = static_cast<uint32_t>(sp[1]);
    const uint32_t iovs_ptr     = static_cast<uint32_t>(sp[2]);
    const uint32_t iovs_len     = static_cast<uint32_t>(sp[3]);
    const uint32_t nwritten_ptr = static_cast<uint32_t>(sp[4]);
    (void)fd;

    uint32_t mem_size = 0;
    uint8_t* mem = m3_GetMemory(runtime, &mem_size, 0);

    uint32_t total_written = 0;
    if (mem)
    {
        std::string line;
        for (uint32_t i = 0; i < iovs_len; ++i)
        {
            uint32_t buf_ptr = 0, buf_len = 0;
            std::memcpy(&buf_ptr, mem + iovs_ptr + i * 8,     4);
            std::memcpy(&buf_len, mem + iovs_ptr + i * 8 + 4, 4);
            if (buf_ptr + buf_len <= mem_size)
            {
                line.append(reinterpret_cast<const char*>(mem + buf_ptr), buf_len);
                total_written += buf_len;
            }
        }
        // Strip trailing newline — LogInfo adds its own.
        if (!line.empty() && line.back() == '\n') line.pop_back();
        if (!line.empty()) LogInfo("[Wasm] {}", line);
        std::memcpy(mem + nwritten_ptr, &total_written, 4);
    }

    *sp = 0u;  // ESUCCESS
    return nullptr;
}

static void LinkWasiStubs(IM3Module module)
{
    // WASI standard imports
    m3_LinkRawFunction(module, "wasi_snapshot_preview1", "fd_write",
                       "i(iiii)", WasiFdWrite);
    m3_LinkRawFunction(module, "wasi_snapshot_preview1", "fd_read",
                       "i(iiii)", WasiStubEnosys);
    m3_LinkRawFunction(module, "wasi_snapshot_preview1", "fd_close",
                       "i(i)",    WasiStubEnosys);
    m3_LinkRawFunction(module, "wasi_snapshot_preview1", "fd_seek",
                       "i(iIii)", WasiStubEnosys);
    m3_LinkRawFunction(module, "wasi_snapshot_preview1", "clock_time_get",
                       "i(iIi)",  WasiStubEnosys);

    // Emscripten invoke_* wrappers — void return
    m3_LinkRawFunction(module, "env", "invoke_v",        "v(i)",        Invoke_v);
    m3_LinkRawFunction(module, "env", "invoke_vi",       "v(ii)",       Invoke_vi);
    m3_LinkRawFunction(module, "env", "invoke_vii",      "v(iii)",      Invoke_vii);
    m3_LinkRawFunction(module, "env", "invoke_viii",     "v(iiii)",     Invoke_viii);
    m3_LinkRawFunction(module, "env", "invoke_viiii",    "v(iiiii)",    Invoke_viiii);
    m3_LinkRawFunction(module, "env", "invoke_viiiii",   "v(iiiiii)",   Invoke_viiiii);
    m3_LinkRawFunction(module, "env", "invoke_viiiiii",  "v(iiiiiii)",  Invoke_viiiiii);

    // Emscripten invoke_* wrappers — non-void return
    m3_LinkRawFunction(module, "env", "invoke_i",        "i(i)",        Invoke_i);
    m3_LinkRawFunction(module, "env", "invoke_ii",       "i(ii)",       Invoke_ii);
    m3_LinkRawFunction(module, "env", "invoke_iii",      "i(iii)",      Invoke_iii);
    m3_LinkRawFunction(module, "env", "invoke_iiii",     "i(iiii)",     Invoke_iiii);

    // Other env imports Emscripten may emit
    m3_LinkRawFunction(module, "env", "__syscall_dup3",
                       "i(iii)", WasiStubEnosys);

    // _emscripten_throw_longjmp() — called by Emscripten's longjmp emulation.
    // In JS it throws an exception that unwinds to the nearest invoke_* wrapper.
    // In wasm3 (no JS), we return a trap so wasm3 unwinds to the caller of
    // m3_CallV instead of crashing on a null compiled pointer.
    m3_LinkRawFunction(module, "env", "_emscripten_throw_longjmp",
                       "v()", [](IM3Runtime, M3ImportContext*, uint64_t*, void*) -> const void* {
                           // Lua called longjmp — an error occurred inside a lua_pcall.
                           // Returning m3Err_trapAbort causes wasm3 to cleanly unwind to
                           // m3_CallV. CallExport will then call GetLuaErrorMsg() for details.
                           LogWarning("[PhoenixScript] _emscripten_throw_longjmp — Lua error, aborting WASM");
                           return m3Err_trapAbort;
                       });
}


// Write a struct Variant's fields into WASM linear memory at sretPtr.
uint32 WriteStructToWasmMemory(IM3Runtime rt, uint32_t sretPtr, const Variant& val, const TypeDescriptor& desc)
{
    uint32_t memSize = 0;
    uint8_t* mem = m3_GetMemory(rt, &memSize, 0);
    if (!mem)
    {
        return 0;
    }

    uint32_t offset = 0;
    for (const auto& field : desc.GetFields() | std::views::values)
    {
        if (field.GetType() == &desc || field.IsStatic() || field.IsScriptHidden())
        {
            continue;
        }

        const TypeDescriptor* type = field.GetType();
        assert(type);

        char c;
        if (Script::ToWasmTypeChar(*type, c))
        {
            Variant fieldGv(*type);
            field.Get(val.GetData(), fieldGv.GetData());
            uint64_t slot = 0;
            Script::WriteWasmReturn(&slot, fieldGv);
            const size_t sz = (c == 'I' || c == 'F') ? 8 : 4;
            if (sretPtr + offset + sz <= memSize)
            {
                std::memcpy(mem + sretPtr + offset, &slot, sz);
            }
            offset += static_cast<uint32_t>(sz);
        }
        else if (Script::IsExpandableStruct(*type))
        {
            // Nested struct — read out of parent buffer, recurse.
            Variant nested(*type);
            field.Get(val.GetData(), nested.GetData());
            offset += WriteStructToWasmMemory(rt, sretPtr + offset, nested, *type);
        }
    }

    return offset;
}

// ── File-local trampoline ─────────────────────────────────────────────────────
//
// wasm3 calls this for every registered host import.
// It rebuilds GenericValue args from the WASM stack, dispatches via Execute(),
// and writes any return value back into the return slot.

static const void* WasmHostTrampoline(IM3Runtime rt, M3ImportContext* ctx, uint64_t* sp, void* /*mem*/)
{
    auto* callCtx = static_cast<WasmEnvironment::CallCtx*>(ctx->userdata);
    WasmEnvironment* self = callCtx->Runtime;
    const MethodDescriptor& fn = callCtx->Descriptor;

#if PHX_WASM_VERBOSE
    LogInfo("[PhoenixScript] >> {}", fn.GetName());
#endif

    const TypeDescriptor* worldDesc = &TypeRegistry::Get<World>();

    std::vector<Variant> args;
    args.reserve(fn.GetParams().size());

    const TypeDescriptor& returnType = *fn.GetReturnType();

    // wasm3: sp[0..numReturns-1] = return slots, sp[numReturns..] = args.
    // Struct returns use sret convention: return type is 'v' (sp[0] is first arg slot),
    // and the actual first WASM arg is an i32 pointer into linear memory.
    const bool returnIsStruct = Script::IsExpandableStruct(returnType);
    const bool returnIsVoid = returnType.GetTypeId() == StaticTypeName<void>::TypeId;
    const bool hasScalarReturn = !returnIsStruct && !returnIsVoid;
    uint64_t* argSp = hasScalarReturn ? (sp + 1) : sp;

    // Consume sret pointer (first WASM arg for struct-returning functions).
    uint32_t sretPtr = 0;
    if (returnIsStruct)
    {
        sretPtr = static_cast<uint32_t>(*argSp++);
    }

    for (const auto& param : fn.GetParams())
    {
        if (param.Type == worldDesc)
        {
            // World param is implicit in per-world runtimes — inject directly.
            if (World* w = self->GetWorld())
            {
                args.push_back(Variant(static_cast<WorldRef>(*w)));
            }
            else
            {
                args.push_back(Variant::Void());
            }
        }
        else if (Script::IsExpandableStruct(*param.Type))
        {
            // Struct param: fields are flattened on the WASM stack, reassemble.
            args.push_back(Script::ReadStructArg(argSp, *param.Type));
        }
        else if (Script::IsSupportedWasmType(*param.Type))
        {
            // Primitive param: read from WASM stack slot (advances argSp).
            args.push_back(Script::ReadWasmArg(argSp, *param.Type));
        }
        else
        {
            // Unknown/unsupported param: inject Void.
            Variant defaultParam(*param.Type);
            args.push_back(defaultParam);
        }
    }

#if PHX_WASM_VERBOSE
    LogInfo("[PhoenixScript] executing {}...", fn.GetName());
#endif

    Variant result = fn.Execute(nullptr, args);

#if PHX_WASM_VERBOSE
    LogInfo("[PhoenixScript] done executing {}", fn.GetName());
#endif

    if (returnIsStruct)
    {
        WriteStructToWasmMemory(rt, sretPtr, result, *fn.GetReturnType());
    }
    else if (hasScalarReturn)
    {
        Script::WriteWasmReturn(sp, result);
    }

    return nullptr;  // m3Err_none
}

// ── WasmWorldRuntime ──────────────────────────────────────────────────────────

WasmEnvironment::WasmEnvironment(
    Session* session,
    World* world,
    const std::shared_ptr<WasmRuntime>& wasmRuntime)
    : ScriptSession(session)
    , ScriptWorld(world)
    , ScriptRuntime(wasmRuntime)
{
    const std::vector<uint8>& wasmBytes = wasmRuntime->GetWasmBytes();
    if (wasmBytes.empty())
    {
        LogError("[PhoenixScript] No WASM bytes to load.");
        return;
    }

    // ── Create wasm3 environment and runtime ──────────────────────────────────

    IM3Environment env = m3_NewEnvironment();
    if (!env)
    {
        LogError("[PhoenixScript] m3_NewEnvironment failed.");
        return;
    }
    Env = env;

    // 512 KB evaluation stack.
    // 64 KB is insufficient when the WASM module embeds the full Lua 5.4
    // interpreter: luaL_newstate + four library opens + luaL_loadbuffer +
    // lua_pcall together create a deep WASM call chain that overflows 64 KB,
    // silently corrupting adjacent heap pages (wasm3 code pages that store
    // IM3Operation pointers), producing a DEP violation on the next callback.
    constexpr uint32_t kStackBytes = 512u * 1024u;
    IM3Runtime runtime = m3_NewRuntime(env, kStackBytes, this);
    if (!runtime)
    {
        LogError("[PhoenixScript] m3_NewRuntime failed.");
        return;
    }
    Runtime = runtime;

    // ── Parse module ─────────────────────────────────────────────────────────

    IM3Module module = nullptr;
    M3Result err = m3_ParseModule(env, &module, wasmBytes.data(), static_cast<uint32_t>(wasmBytes.size()));
    if (err)
    {
        LogError("[PhoenixScript] m3_ParseModule: {}", err);
        return;
    }

    // ── Load module ───────────────────────────────────────────────────────────
    //
    // m3_LoadModule attaches the module to the runtime (sets module->runtime)
    // and initializes memory/globals.  It does NOT run the WASM start section;
    // that happens lazily on the first m3_FindFunction call.
    //
    // Imports must be linked AFTER this call because m3_LinkRawFunctionEx
    // allocates wasm3 code pages via the runtime, which requires module->runtime
    // to be non-null.

    err = m3_LoadModule(runtime, module);
    if (err)
    {
        LogError("[PhoenixScript] m3_LoadModule: {}", err);
        m3_FreeModule(module);
        return;
    }
    // module ownership transferred to runtime — do not free it separately

    // ── Link ALL imports AFTER LoadModule, BEFORE first FindFunction call ─────
    //
    // emscripten's __wasm_call_ctors start section fires on the first
    // m3_FindFunction call.  It can invoke WASI functions (fd_write, etc.)
    // and, once the Lua state is initialized, will call through to Phoenix
    // host functions.  All imports must be linked now so the start section
    // finds them resolved rather than unlinked (→ "missing imported function").
    //
    // 1. WASI / Emscripten env stubs (fd_write, clock_time_get, …)
    // 2. Phoenix host-function trampolines
    //
    // CallContexts must be fully populated before any pointer reaches wasm3;
    // reserve() guarantees the vector never reallocates after this point.

    LinkWasiStubs(module);

    const auto& hostEntries = wasmRuntime->GetRegistrations();
    CallContexts.reserve(hostEntries.size());
    for (const auto& entry : hostEntries)
    {
        CallContexts.push_back({ this, entry.Method });
    }

    for (size_t i = 0; i < hostEntries.size(); ++i)
    {
        const WasmHostEntry& entry = hostEntries[i];
        const std::string    sig   = Script::BuildWasmSignature(entry.Method);
        const char*          mod   = entry.ImportModule.empty() ? "*" : entry.ImportModule.c_str();

        M3Result result = m3_LinkRawFunctionEx(module,
            mod,
            entry.Method.GetName().c_str(),
            sig.c_str(),
            &WasmHostTrampoline,
            &CallContexts[i]);

        // functionLookupFailed is normal — the WASM just doesn't call this function.
        // Any other error (e.g. type mismatch) means the import exists but the
        // host signature disagrees with what the WASM expects.
        if (result && result != m3Err_functionLookupFailed)
        {
            LogError("[PhoenixScript] Link mismatch {}::{} host_sig={} — {}",
                mod, entry.Method.GetName(), sig, result);
        }
#if PHX_WASM_VERBOSE
        else
        {
            const char* status = result ? "skipped" : "linked";
            LogVerbose("[PhoenixScript] {} {}::{} {}", status, mod, entry.Method.GetName(), sig);
        }
#endif
    }

    // ── Call _initialize ─────────────────────────────────────────────────────
    //
    // Emscripten -sSTANDALONE_WASM=1 does NOT emit a WASM start section; instead
    // it exports "_initialize" which calls __wasm_call_ctors (C global ctors,
    // Emscripten's internal heap/stack setup).  wasm3 never triggers this
    // automatically.  Without calling it, Emscripten's __stack_pointer global
    // and memory allocator are uninitialized — the first WASM memory access then
    // AV's.  Call it now, after all imports are linked but before any script
    // exports (OnWorldInitialize, etc.) are invoked.
    //
    // "_initialize" may not be exported (e.g. plain C WASM without Emscripten),
    // so we only call it if it exists.

    IM3Function initFn = nullptr;
    if (m3_FindFunction(&initFn, runtime, "_initialize") == nullptr)
    {
        LogInfo("[PhoenixScript] Calling _initialize...");
        const M3Result initErr = m3_CallV(initFn);
        if (initErr)
        {
            LogError("[PhoenixScript] _initialize failed: {}", initErr);
        }
        else
        {
            LogInfo("[PhoenixScript] _initialize OK");
        }
    }
    else
    {
        LogInfo("[PhoenixScript] No _initialize export — skipping");
    }
}

WasmEnvironment::~WasmEnvironment()
{
    // IM3Runtime owns the module — free the runtime first, then the environment.
    if (ScriptRuntime)
    {
        m3_FreeRuntime(static_cast<IM3Runtime>(Runtime));
        ScriptRuntime = nullptr;
    }
    if (Env)
    {
        m3_FreeEnvironment(static_cast<IM3Environment>(Env));
        Env = nullptr;
    }
}

bool WasmEnvironment::CallExport(const char* name, int argc, const void** argPtrs, int retc, const void** retPtrs)
{
    auto* fn = static_cast<IM3Function>(FindExport(name));
    if (!fn)
    {
        return false;
    }

    IM3Runtime fnRuntime = fn->module ? fn->module->runtime : nullptr;

#if PHX_WASM_VERBOSE
    LogVerbose("[PhoenixScript] Calling '{}' argc={}", name, argc);
#endif

#ifdef _WIN32
    ULONG stackGuarantee = 65536;
    SetThreadStackGuarantee(&stackGuarantee);

    static DWORD s_lastExceptionCode = 0;
    M3Result err = nullptr;
    __try
    {
        err = m3_Call(fn, argc, argPtrs);
    }
    __except(
        [](EXCEPTION_POINTERS* ep) -> int
        {
            s_lastExceptionCode = ep->ExceptionRecord->ExceptionCode;
            if (s_lastExceptionCode == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
            {
                fprintf(stderr, "[SEH] ACCESS_VIOLATION %s addr=0x%llX\n",
                        ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "read" : "write",
                        (unsigned long long)ep->ExceptionRecord->ExceptionInformation[1]);
                fflush(stderr);
            }
            return EXCEPTION_EXECUTE_HANDLER;
        }
        (GetExceptionInformation()))
    {
        LogError("[PhoenixScript] SEH exception 0x{:08X} ({}) calling '{}'",
                 s_lastExceptionCode,
                 s_lastExceptionCode == 0xC0000005 ? "ACCESS_VIOLATION" :
                 s_lastExceptionCode == 0xC00000FD ? "STACK_OVERFLOW" :
                 s_lastExceptionCode == 0xC0000374 ? "HEAP_CORRUPTION" : "other",
                 name);
        if (s_lastExceptionCode == 0xC00000FD)
        {
            _resetstkoflw();
        }

        if (fnRuntime)
        {
            IM3BacktraceInfo bt = m3_GetBacktrace(fnRuntime);
            if (bt)
            {
                LogError("[PhoenixScript] wasm3 backtrace:");
                IM3BacktraceFrame frame = bt->frames;
                int depth = 0;
                while (frame && frame != M3_BACKTRACE_TRUNCATED && depth < 32)
                {
                    const char* fname = "?";
                    if (frame->function && frame->function->numNames > 0 && frame->function->names[0])
                        fname = frame->function->names[0];
                    LogError("  [{}] +0x{:04X} {}", depth, frame->moduleOffset, fname);
                    frame = frame->next;
                    ++depth;
                }
                if (frame == M3_BACKTRACE_TRUNCATED)
                    LogError("  ... (truncated)");
            }
            else
            {
                M3ErrorInfo ei{};
                m3_GetErrorInfo(fnRuntime, &ei);
                if (ei.message)
                    LogError("[PhoenixScript] wasm3 error: {} ({}:{})", ei.message, ei.file ? ei.file : "?", ei.line);
            }
        }
        return false;
    }
#else
    const M3Result err = m3_Call(fn, argc, argPtrs);
#endif

    if (err)
    {
        LogError("[PhoenixScript] Error calling '{}': {}", name, err);

        if (fnRuntime)
        {
            M3ErrorInfo ei{};
            m3_GetErrorInfo(fnRuntime, &ei);
            if (ei.message && ei.message != err)
            {
                LogError("[PhoenixScript] wasm3 detail: {} ({}:{})", ei.message, ei.file ? ei.file : "?", ei.line);
            }
        }

        if (fnRuntime)
        {
            IM3Function getErrFn = nullptr;
            if (m3_FindFunction(&getErrFn, fnRuntime, "GetLuaErrorMsg") == m3Err_none && getErrFn)
            {
                if (m3_Call(getErrFn, 0, nullptr) == m3Err_none)
                {
                    uint32_t wasmPtr = 0;
                    const void* retPtrs_[] = { &wasmPtr };
                    m3_GetResults(getErrFn, 1, retPtrs_);
                    uint32_t memSize = 0;
                    const uint8_t* mem = m3_GetMemory(fnRuntime, &memSize, 0);
                    if (wasmPtr && mem && wasmPtr < memSize)
                    {
                        LogError("[PhoenixScript] Lua error: {}", reinterpret_cast<const char*>(mem + wasmPtr));
                    }
                }
            }
        }
        return false;
    }

    if (retc > 0 && retPtrs)
    {
        m3_GetResults(fn, retc, retPtrs);
    }

#if PHX_WASM_VERBOSE
    LogVerbose("[PhoenixScript] '{}' OK", name);
#endif
    return true;
}

uint8_t* WasmEnvironment::GetMemory(uint32_t* outSize) const
{
    uint32_t size = 0;
    uint8_t* mem = m3_GetMemory(static_cast<IM3Runtime>(Runtime), &size, 0);
    if (outSize)
    {
        *outSize = size;
    }
    return mem;
}

void WasmEnvironment::Snapshot()
{
    if (!ScriptRuntime) return;

    uint32_t size = 0;
    const uint8_t* mem = m3_GetMemory(static_cast<IM3Runtime>(Runtime), &size, 0);
    if (!mem || size == 0) return;

    MemorySnapshot.resize(size);
    std::memcpy(MemorySnapshot.data(), mem, size);
}

void WasmEnvironment::Restore()
{
    if (!ScriptRuntime || MemorySnapshot.empty()) return;

    uint32_t size = 0;
    uint8_t* mem = m3_GetMemory(static_cast<IM3Runtime>(Runtime), &size, 0);
    if (!mem || size == 0) return;

    const uint32_t restoreSize = static_cast<uint32_t>(std::min(MemorySnapshot.size(), static_cast<size_t>(size)));
    std::memcpy(mem, MemorySnapshot.data(), restoreSize);
}

void* WasmEnvironment::FindExport(const char* name)
{
    auto it = ExportCache.find(name);
    if (it != ExportCache.end())
    {
        return it->second;
    }

    IM3Function fn = nullptr;
    const M3Result err = m3_FindFunction(&fn, static_cast<IM3Runtime>(Runtime), name);
    void* result = err ? nullptr : static_cast<void*>(fn);
    ExportCache.emplace(name, result);
    return result;
}
