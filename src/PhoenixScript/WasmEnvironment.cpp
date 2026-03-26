#include "PhoenixScript/WasmEnvironment.h"

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
#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Reflection/GenericValue.h"
#include "PhoenixSim/Worlds.h"
#include "PhoenixSim/Scripting/IScriptRuntime.h"

using namespace Phoenix;

// Set to 1 to log every host→WASM import link attempt and unresolved imports.
// Leave at 0 for normal builds — signature mismatches are always logged.
#define PHX_WASM_VERBOSE 1

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
        if (!line.empty()) LogInfo("[Lua] {}", line);
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
                           // m3_CallV. CallVoid will then call GetLuaErrorMsg() for details.
                           LogWarning("[PhoenixScript] _emscripten_throw_longjmp — Lua error, aborting WASM");
                           return m3Err_trapAbort;
                       });
}

// ── Struct marshaling helpers ─────────────────────────────────────────────────

// Returns properties of desc sorted alphabetically — provides a deterministic
// field order that matches the PhoenixWasmGen generator.
static std::vector<std::pair<std::string, const PropertyDescriptor*>>
GetSortedProps(const TypeDescriptor& desc)
{
    std::vector<std::pair<std::string, const PropertyDescriptor*>> sorted;
    for (const auto& [name, prop] : desc.GetProperties())
        sorted.push_back({ name, &prop });
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    return sorted;
}

// Appends flattened WASM type chars for all fields of a struct (sorted alphabetically).
static void AppendStructFieldChars(std::string& sig, const TypeDescriptor& desc)
{
    for (const auto& [name, prop] : GetSortedProps(desc))
    {
        GenericValueTypeRef tr = prop->GetTypeRef();
        if (tr.IsStruct() && tr.Descriptor)
            AppendStructFieldChars(sig, *tr.Descriptor);  // nested struct
        else
        {
            char c = WasmEnvironment::ToWasmTypeChar(tr);
            if (c != 'v') sig += c;
        }
    }
}

// Returns true when a type is a struct small enough to marshal over WASM.
static bool IsExpandableStruct(const GenericValueTypeRef& type)
{
    return type.IsStruct() && type.Descriptor &&
           type.Descriptor->GetSize() <= sizeof(std::byte[32]);
}

// Read struct fields from the WASM stack into a GenericValue.
static GenericValue ReadStructArg(uint64_t*& sp, const TypeDescriptor& desc)
{
    GenericValue result;
    result.Type = { EGenericValueType::Unknown, &desc };
    desc.DefaultConstruct(result.Buffer);

    for (const auto& [name, prop] : GetSortedProps(desc))
    {
        GenericValueTypeRef tr = prop->GetTypeRef();
        if (tr.IsStruct() && tr.Descriptor)
        {
            // Nested struct — read its fields recursively, then copy bytes into parent.
            GenericValue nested = ReadStructArg(sp, *tr.Descriptor);
            prop->PropertyAccessor->Set(result.Buffer, nested.Buffer, sizeof(nested.Buffer));
        }
        else if (WasmEnvironment::ToWasmTypeChar(tr) != 'v')
        {
            GenericValue fieldGv = WasmEnvironment::ReadWasmArg(sp, tr);
            prop->PropertyAccessor->Set(result.Buffer, fieldGv.Buffer, sizeof(fieldGv.Buffer));
        }
    }
    return result;
}

// Write a struct GenericValue's fields into WASM linear memory at sretPtr.
// Fields are written in alphabetical order (matching the generator).
static void WriteStructToWasmMemory(IM3Runtime rt, uint32_t sretPtr,
                                     const GenericValue& val, const TypeDescriptor& desc)
{
    uint32_t memSize = 0;
    uint8_t* mem = m3_GetMemory(rt, &memSize, 0);
    if (!mem) return;

    uint32_t offset = 0;
    for (const auto& [name, prop] : GetSortedProps(desc))
    {
        GenericValueTypeRef tr = prop->GetTypeRef();
        if (tr.IsStruct() && tr.Descriptor)
        {
            // Nested struct — read out of parent buffer, recurse.
            GenericValue nested;
            nested.Type = tr;
            prop->PropertyAccessor->Get(val.Buffer, nested.Buffer, sizeof(nested.Buffer));
            // Write each nested field sequentially.
            for (const auto& [fn2, fp2] : GetSortedProps(*tr.Descriptor))
            {
                GenericValueTypeRef tr2 = fp2->GetTypeRef();
                char c = WasmEnvironment::ToWasmTypeChar(tr2);
                if (c == 'v') continue;
                GenericValue f2;
                f2.Type = tr2;
                fp2->PropertyAccessor->Get(nested.Buffer, f2.Buffer, sizeof(f2.Buffer));
                uint64_t slot = 0;
                WasmEnvironment::WriteWasmReturn(&slot, f2);
                const size_t sz = (c == 'I' || c == 'F') ? 8 : 4;
                if (sretPtr + offset + sz <= memSize)
                    std::memcpy(mem + sretPtr + offset, &slot, sz);
                offset += static_cast<uint32_t>(sz);
            }
        }
        else
        {
            char c = WasmEnvironment::ToWasmTypeChar(tr);
            if (c == 'v') continue;
            GenericValue fieldGv;
            fieldGv.Type = tr;
            prop->PropertyAccessor->Get(val.Buffer, fieldGv.Buffer, sizeof(fieldGv.Buffer));
            uint64_t slot = 0;
            WasmEnvironment::WriteWasmReturn(&slot, fieldGv);
            const size_t sz = (c == 'I' || c == 'F') ? 8 : 4;
            if (sretPtr + offset + sz <= memSize)
                std::memcpy(mem + sretPtr + offset, &slot, sz);
            offset += static_cast<uint32_t>(sz);
        }
    }
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

    const TypeDescriptor* worldDesc = &World::GetStaticTypeDescriptor();

    std::vector<GenericValue> args;
    args.reserve(fn.Params.size());

    // wasm3: sp[0..numReturns-1] = return slots, sp[numReturns..] = args.
    // Struct returns use sret convention: return type is 'v' (sp[0] is first arg slot),
    // and the actual first WASM arg is an i32 pointer into linear memory.
    const bool returnIsStruct = IsExpandableStruct(fn.Return.Type);
    const bool hasScalarReturn = !fn.Return.Type.IsVoid() && !returnIsStruct;
    uint64_t* argSp = hasScalarReturn ? (sp + 1) : sp;

    // Consume sret pointer (first WASM arg for struct-returning functions).
    uint32_t sretPtr = 0;
    if (returnIsStruct)
        sretPtr = static_cast<uint32_t>(*argSp++);

    for (const auto& param : fn.Params)
    {
        if (param.Type.Descriptor == worldDesc)
        {
            // World param is implicit in per-world runtimes — inject directly.
            if (World* w = self->GetWorld())
                args.push_back(GenericConverter<World>::Borrow(*w));
            else
                args.push_back(GenericValue::Void());
        }
        else if (IsExpandableStruct(param.Type))
        {
            // Struct param: fields are flattened on the WASM stack, reassemble.
            args.push_back(ReadStructArg(argSp, *param.Type.Descriptor));
        }
        else if (WasmEnvironment::ToWasmTypeChar(param.Type) != 'v')
        {
            // Primitive param: read from WASM stack slot (advances argSp).
            args.push_back(WasmEnvironment::ReadWasmArg(argSp, param.Type));
        }
        else
        {
            // Unknown/unsupported param: inject Void.
            args.push_back(GenericValue::Void());
        }
    }

    GenericValue result = fn.Execute(nullptr, args);

    if (returnIsStruct)
        WriteStructToWasmMemory(rt, sretPtr, result, *fn.Return.Type.Descriptor);
    else if (hasScalarReturn)
        WasmEnvironment::WriteWasmReturn(sp, result);

    return nullptr;  // m3Err_none
}

// ── Static helpers ────────────────────────────────────────────────────────────

char WasmEnvironment::ToWasmTypeChar(const GenericValueTypeRef& type)
{
    if (type.IsVoid()) return 'v';
    switch (type.Primitive)
    {
    case EGenericValueType::Bool:
    case EGenericValueType::Int8:   case EGenericValueType::UInt8:
    case EGenericValueType::Int16:  case EGenericValueType::UInt16:
    case EGenericValueType::Int32:  case EGenericValueType::UInt32:
    case EGenericValueType::Name:
        return 'i';
    case EGenericValueType::Int64:  case EGenericValueType::UInt64:
        return 'I';
    case EGenericValueType::Float:
        return 'f';
    case EGenericValueType::FixedPoint:
        return 'i';  // TFixed stores int32_t internally, not IEEE 754 float
    case EGenericValueType::Double:
        return 'F';
    case EGenericValueType::Unknown:
    case EGenericValueType::String:
    case EGenericValueType::Struct:
    case EGenericValueType::COUNT:
    default:
        return 'v';  // Struct/Unknown — unsupported over the WASM scalar ABI
    }
}

std::string WasmEnvironment::BuildWasmSignature(const MethodDescriptor& fn)
{
    const TypeDescriptor* worldDesc = &World::GetStaticTypeDescriptor();
    const bool returnIsStruct = IsExpandableStruct(fn.Return.Type);

    std::string sig;
    // Struct return → void return + leading 'i' sret pointer.
    sig += returnIsStruct ? 'v' : ToWasmTypeChar(fn.Return.Type);
    sig += '(';
    if (returnIsStruct)
        sig += 'i';  // sret: pointer into WASM linear memory

    for (const auto& param : fn.Params)
    {
        if (param.Type.Descriptor == worldDesc)
            continue;  // world is injected — not a WASM argument
        if (IsExpandableStruct(param.Type))
            AppendStructFieldChars(sig, *param.Type.Descriptor);
        else
        {
            char c = ToWasmTypeChar(param.Type);
            if (c != 'v') sig += c;
        }
    }
    sig += ')';
    return sig;
}

GenericValue WasmEnvironment::ReadWasmArg(uint64_t*& sp, const GenericValueTypeRef& type)
{
    const uint64_t slot = *sp++;

    switch (type.Primitive)
    {
    case EGenericValueType::Bool:
        return GenericConverter<bool>::Borrow(static_cast<uint32_t>(slot) != 0u);
    case EGenericValueType::Int8:
        return GenericConverter<int8_t>::Borrow(static_cast<int8_t>(static_cast<int32_t>(slot)));
    case EGenericValueType::UInt8:
        return GenericConverter<uint8_t>::Borrow(static_cast<uint8_t>(slot));
    case EGenericValueType::Int16:
        return GenericConverter<int16_t>::Borrow(static_cast<int16_t>(static_cast<int32_t>(slot)));
    case EGenericValueType::UInt16:
        return GenericConverter<uint16_t>::Borrow(static_cast<uint16_t>(slot));
    case EGenericValueType::Int32:
        return GenericConverter<int32_t>::Borrow(static_cast<int32_t>(slot));
    case EGenericValueType::UInt32:
        return GenericConverter<uint32_t>::Borrow(static_cast<uint32_t>(slot));
    case EGenericValueType::Int64:
        return GenericConverter<int64_t>::Borrow(static_cast<int64_t>(slot));
    case EGenericValueType::UInt64:
        return GenericConverter<uint64_t>::Borrow(slot);
    case EGenericValueType::Float:
    {
        float v;
        const uint32_t u = static_cast<uint32_t>(slot);
        std::memcpy(&v, &u, sizeof(float));
        return GenericConverter<float>::Borrow(v);
    }
    case EGenericValueType::FixedPoint:
    {
        // TFixed stores a raw int32_t. Pass it through as-is.
        GenericValue gv;
        gv.Type.Primitive = EGenericValueType::FixedPoint;
        const int32_t raw = static_cast<int32_t>(slot);
        std::memcpy(gv.Buffer, &raw, sizeof(int32_t));
        return gv;
    }
    case EGenericValueType::Double:
    {
        double v;
        std::memcpy(&v, &slot, sizeof(double));
        return GenericConverter<double>::Borrow(v);
    }
    case EGenericValueType::Name:
    {
        const FName name(static_cast<uint32_t>(slot));
        return GenericConverter<FName>::Borrow(name);
    }
    case EGenericValueType::Unknown:
    case EGenericValueType::String:
    case EGenericValueType::Struct:
    case EGenericValueType::COUNT:
    default:
        return GenericValue::Void();
    }
}

void WasmEnvironment::WriteWasmReturn(uint64_t* sp, const GenericValue& val)
{
    switch (val.Type.Primitive)
    {
    case EGenericValueType::Bool:
        *sp = val.As<bool>() ? 1u : 0u;
        break;
    case EGenericValueType::Int8:
        *sp = static_cast<uint64_t>(static_cast<int32_t>(val.As<int8_t>()));
        break;
    case EGenericValueType::UInt8:
        *sp = static_cast<uint64_t>(val.As<uint8_t>());
        break;
    case EGenericValueType::Int16:
        *sp = static_cast<uint64_t>(static_cast<int32_t>(val.As<int16_t>()));
        break;
    case EGenericValueType::UInt16:
        *sp = static_cast<uint64_t>(val.As<uint16_t>());
        break;
    case EGenericValueType::Int32:
        *sp = static_cast<uint64_t>(val.As<int32_t>());
        break;
    case EGenericValueType::UInt32:
        *sp = static_cast<uint64_t>(val.As<uint32_t>());
        break;
    case EGenericValueType::Int64:
        *sp = static_cast<uint64_t>(val.As<int64_t>());
        break;
    case EGenericValueType::UInt64:
        *sp = val.As<uint64_t>();
        break;
    case EGenericValueType::Float:
    {
        const float v = val.As<float>();
        uint32_t u;
        std::memcpy(&u, &v, sizeof(float));
        *sp = u;
        break;
    }
    case EGenericValueType::FixedPoint:
    {
        // Raw int32 bits — pass through without float reinterpretation.
        int32_t raw;
        std::memcpy(&raw, val.Buffer, sizeof(int32_t));
        *sp = static_cast<uint64_t>(static_cast<uint32_t>(raw));
        break;
    }
    case EGenericValueType::Double:
    {
        const double v = val.As<double>();
        std::memcpy(sp, &v, sizeof(double));
        break;
    }
    case EGenericValueType::Name:
    {
        const FName n = val.As<FName>();
        *sp = static_cast<uint64_t>(static_cast<hash32_t>(n));
        break;
    }
    case EGenericValueType::Unknown:
    case EGenericValueType::String:
    case EGenericValueType::Struct:
    case EGenericValueType::COUNT:
        break;
    }
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
    M3Result err = m3_ParseModule(env, &module,
        wasmBytes.data(), static_cast<uint32_t>(wasmBytes.size()));
    if (err)
    {
        LogError("[PhoenixScript] m3_ParseModule: {}", err);
        return;
    }

    // ── Load module ───────────────────────────────────────────────────────────
    //
    // m3_LoadModule attaches the module to the runtime (sets module->runtime)
    // and initialises memory/globals.  It does NOT run the WASM start section;
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
    // and, once the Lua state is initialised, will call through to Phoenix
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
        CallContexts.push_back({ this, entry.Descriptor });

    for (size_t i = 0; i < hostEntries.size(); ++i)
    {
        const WasmHostEntry& entry = hostEntries[i];
        const std::string    sig   = BuildWasmSignature(entry.Descriptor);
        const char*          mod   = entry.ImportModule.empty() ? "*" : entry.ImportModule.c_str();

        M3Result result = m3_LinkRawFunctionEx(module,
            mod,
            entry.Descriptor.Name.c_str(),
            sig.c_str(),
            &WasmHostTrampoline,
            &CallContexts[i]);

        // functionLookupFailed is normal — the WASM just doesn't call this function.
        // Any other error (e.g. type mismatch) means the import exists but the
        // host signature disagrees with what the WASM expects.
        if (result && result != m3Err_functionLookupFailed)
        {
            LogError("[PhoenixScript] Link mismatch {}::{} host_sig={} — {}",
                mod, entry.Descriptor.Name, sig, result);
        }
#if PHX_WASM_VERBOSE
        else
        {
            const char* status = result ? "skipped" : "linked";
            LogVerbose("[PhoenixScript] {} {}::{} {}", status, mod, entry.Descriptor.Name, sig);
        }
#endif
    }

    // ── Call _initialize ─────────────────────────────────────────────────────
    //
    // Emscripten -sSTANDALONE_WASM=1 does NOT emit a WASM start section; instead
    // it exports "_initialize" which calls __wasm_call_ctors (C global ctors,
    // Emscripten's internal heap/stack setup).  wasm3 never triggers this
    // automatically.  Without calling it, Emscripten's __stack_pointer global
    // and memory allocator are uninitialised — the first WASM memory access then
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
            LogError("[PhoenixScript] _initialize failed: {}", initErr);
        else
            LogInfo("[PhoenixScript] _initialize OK");
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

bool WasmEnvironment::LoadLuaScript(const std::filesystem::path& path)
{
    // Read the .lua file.
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        LogError("[PhoenixScript] Cannot open Lua script: {}", path.string());
        return false;
    }
    std::vector<char> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>{});

    // Locate the GetScriptBuffer() and LoadScript(i32) WASM exports.
    IM3Function getBuf    = static_cast<IM3Function>(FindExport("GetScriptBuffer"));
    IM3Function loadScript = static_cast<IM3Function>(FindExport("LoadScript"));
    if (!getBuf || !loadScript)
    {
        LogError("[PhoenixScript] WASM missing GetScriptBuffer/LoadScript exports — not a lua_wasm binary");
        return false;
    }

    // Call GetScriptBuffer() → i32 (WASM linear-memory address of the buffer).
    if (m3_Call(getBuf, 0, nullptr))
    {
        LogError("[PhoenixScript] GetScriptBuffer() call failed");
        return false;
    }
    uint32_t wasmPtr = 0;
    {
        const void* retPtrs[] = { &wasmPtr };
        m3_GetResults(getBuf, 1, retPtrs);
    }

    // Bounds-check: script must fit in the WASM buffer.
    uint32_t memSize = 0;
    uint8_t* mem = m3_GetMemory(static_cast<IM3Runtime>(Runtime), &memSize, 0);
    if (!mem || wasmPtr + bytes.size() > memSize)
    {
        LogError("[PhoenixScript] Lua script ({} bytes) exceeds WASM buffer at offset {}",
                 static_cast<uint32_t>(bytes.size()), wasmPtr);
        return false;
    }

    // Copy script bytes into WASM linear memory.
    std::memcpy(mem + wasmPtr, bytes.data(), bytes.size());

    // Call LoadScript(i32 len) → i32.
    const int32_t len = static_cast<int32_t>(bytes.size());
    {
        const void* argPtrs[] = { &len };
        if (m3_Call(loadScript, 1, argPtrs))
        {
            LogError("[PhoenixScript] LoadScript() call failed");
            return false;
        }
        int32_t result = -1;
        const void* retPtrs[] = { &result };
        m3_GetResults(loadScript, 1, retPtrs);
        if (result != 0)
        {
            LogError("[PhoenixScript] LoadScript() returned {} — script too large?", result);
            return false;
        }
    }

    LuaScriptPath = path;
    LogInfo("[PhoenixScript] Loaded Lua script: {} ({} bytes)", path.string(), static_cast<uint32_t>(bytes.size()));
    return true;
}

bool WasmEnvironment::ReloadLuaScript()
{
    if (LuaScriptPath.empty())
    {
        LogWarning("[PhoenixScript] ReloadLuaScript: no script loaded yet");
        return false;
    }
    return LoadLuaScript(LuaScriptPath);
}

bool WasmEnvironment::RunString(const std::string& code)
{
    IM3Function getBuf   = static_cast<IM3Function>(FindExport("GetScriptBuffer"));
    IM3Function runStrFn = static_cast<IM3Function>(FindExport("RunString"));
    if (!getBuf || !runStrFn)
    {
        LogError("[PhoenixScript] WASM missing GetScriptBuffer/RunString exports");
        return false;
    }

    // Get the WASM buffer address.
    if (m3_Call(getBuf, 0, nullptr))
        return false;
    uint32_t wasmPtr = 0;
    { const void* r[] = { &wasmPtr }; m3_GetResults(getBuf, 1, r); }

    // Bounds-check.
    uint32_t memSize = 0;
    uint8_t* mem = m3_GetMemory(static_cast<IM3Runtime>(Runtime), &memSize, 0);
    if (!mem || wasmPtr + code.size() > memSize)
    {
        LogError("[PhoenixScript] RunString: code ({} bytes) exceeds WASM buffer", code.size());
        return false;
    }

    std::memcpy(mem + wasmPtr, code.data(), code.size());

    const int32_t len = static_cast<int32_t>(code.size());
    { const void* a[] = { &len }; m3_Call(runStrFn, 1, a); }
    int32_t result = -1;
    { const void* r[] = { &result }; m3_GetResults(runStrFn, 1, r); }
    if (result != 0)
    {
        LogError("[PhoenixScript] RunString returned {} — Lua error (see console)", result);
        return false;
    }
    return true;
}

bool WasmEnvironment::CallVoid(const char* name)
{
    auto* fn = static_cast<IM3Function>(FindExport(name));
    if (!fn)
        return false;

    IM3Runtime fnRuntime = fn->module ? fn->module->runtime : nullptr;
    LogInfo("Calling wasm function: {} compiled={}", name, static_cast<const void*>(fn->compiled));

#ifdef _WIN32
    // Reserve 64 KB of stack for the exception handler itself so we can catch
    // stack-overflow exceptions (0xC00000FD), which need guaranteed stack space.
    ULONG stackGuarantee = 65536;
    SetThreadStackGuarantee(&stackGuarantee);

    static DWORD s_lastExceptionCode = 0;
    M3Result err = nullptr;
    __try {
        err = m3_CallV(fn);
    } __except([](EXCEPTION_POINTERS* ep) -> int {
            s_lastExceptionCode = ep->ExceptionRecord->ExceptionCode;
            if (s_lastExceptionCode == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
            {
                fprintf(stderr, "[SEH] ACCESS_VIOLATION %s addr=0x%llX\n",
                        ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "read" : "write",
                        (unsigned long long)ep->ExceptionRecord->ExceptionInformation[1]);
                fflush(stderr);
            }
            return EXCEPTION_EXECUTE_HANDLER;
        }(GetExceptionInformation())) {
        LogError("[PhoenixScript] SEH exception 0x{:08X} ({}) calling '{}'",
                 s_lastExceptionCode,
                 s_lastExceptionCode == 0xC0000005 ? "ACCESS_VIOLATION" :
                 s_lastExceptionCode == 0xC00000FD ? "STACK_OVERFLOW" :
                 s_lastExceptionCode == 0xC0000374 ? "HEAP_CORRUPTION" : "other",
                 name);
        if (s_lastExceptionCode == 0xC00000FD)
            _resetstkoflw();

        // Dump wasm3 backtrace
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
                // Backtrace not available — at least dump the error info
                M3ErrorInfo ei{};
                m3_GetErrorInfo(fnRuntime, &ei);
                if (ei.message)
                    LogError("[PhoenixScript] wasm3 error: {} ({}:{})", ei.message, ei.file ? ei.file : "?", ei.line);
            }
        }
        return false;
    }
#else
    const M3Result err = m3_CallV(fn);
#endif
    if (err)
    {
        LogError("[PhoenixScript] Error calling '{}': {}", name, err);

        // If this was a Lua longjmp trap, try to retrieve the Lua error message.
        // GetLuaErrorMsg() reads the top of the Lua stack and writes it to a buffer
        // in WASM linear memory, then returns the WASM-side pointer to that buffer.
        if (fnRuntime)
        {
            IM3Function getErrFn = nullptr;
            if (m3_FindFunction(&getErrFn, fnRuntime, "GetLuaErrorMsg") == m3Err_none && getErrFn)
            {
                const M3Result callRes = m3_Call(getErrFn, 0, nullptr);
                if (!callRes)
                {
                    uint32_t wasmPtr = 0;
                    const void* retPtrs[] = { &wasmPtr };
                    m3_GetResults(getErrFn, 1, retPtrs);

                    uint32_t memSize = 0;
                    const uint8_t* mem = m3_GetMemory(fnRuntime, &memSize, 0);
                    if (wasmPtr && mem && wasmPtr < memSize)
                        LogError("[PhoenixScript] Lua error message: {}",
                                 reinterpret_cast<const char*>(mem + wasmPtr));
                }
            }
        }
    }
    else
    {
        LogInfo("[PhoenixScript] '{}' returned OK", name);
    }
    return err == nullptr;
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

    const uint32_t restoreSize =
        static_cast<uint32_t>(std::min(MemorySnapshot.size(), static_cast<size_t>(size)));
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
