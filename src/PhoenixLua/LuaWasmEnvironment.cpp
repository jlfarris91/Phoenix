
#include "PhoenixLua/LuaWasmEnvironment.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

#include "PhoenixSim/Logging.h"

using namespace Phoenix;

LuaWasmEnvironment::LuaWasmEnvironment(Session* session, World* world,
                                        const std::shared_ptr<WasmRuntime>& runtime)
    : WasmEnvironment(session, world, runtime)
{
}

bool LuaWasmEnvironment::LoadLuaScript(const std::filesystem::path& path)
{
    // Read the .lua file.
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        LogError("[FeatureLua] Cannot open Lua script: {}", path.string());
        return false;
    }
    std::vector<char> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>{});

    // Call GetScriptBuffer() → i32 (WASM linear-memory address of the script buffer).
    uint32_t wasmPtr = 0;
    {
        const void* retPtrs[] = { &wasmPtr };
        if (!CallExport("GetScriptBuffer", 0, nullptr, 1, retPtrs))
        {
            LogError("[FeatureLua] WASM missing GetScriptBuffer export — not a lua.wasm binary");
            return false;
        }
    }

    // Bounds-check: script must fit in the WASM buffer.
    uint32_t memSize = 0;
    uint8_t* mem = GetMemory(&memSize);
    if (!mem || static_cast<uint64_t>(wasmPtr) + bytes.size() > memSize)
    {
        LogError("[FeatureLua] Lua script ({} bytes) exceeds WASM buffer at offset {}",
                 static_cast<uint32_t>(bytes.size()), wasmPtr);
        return false;
    }

    // Copy script bytes into WASM linear memory.
    std::memcpy(mem + wasmPtr, bytes.data(), bytes.size());

    // Call LoadScript(i32 len) → i32.
    const int32_t len = static_cast<int32_t>(bytes.size());
    int32_t result = -1;
    {
        const void* argPtrs[] = { &len };
        const void* retPtrs[] = { &result };
        if (!CallExport("LoadScript", 1, argPtrs, 1, retPtrs))
        {
            LogError("[FeatureLua] LoadScript export call failed");
            return false;
        }
    }
    if (result != 0)
    {
        LogError("[FeatureLua] LoadScript returned {} — script too large?", result);
        return false;
    }

    LuaScriptPath = path;
    LogInfo("[FeatureLua] Loaded Lua script: {} ({} bytes)",
            path.string(), static_cast<uint32_t>(bytes.size()));
    return true;
}

bool LuaWasmEnvironment::ReloadLuaScript()
{
    if (LuaScriptPath.empty())
    {
        LogWarning("[FeatureLua] ReloadLuaScript: no script loaded yet");
        return false;
    }
    return LoadLuaScript(LuaScriptPath);
}

bool LuaWasmEnvironment::RunString(const std::string& code)
{
    // Get the WASM script buffer address.
    uint32_t wasmPtr = 0;
    {
        const void* retPtrs[] = { &wasmPtr };
        if (!CallExport("GetScriptBuffer", 0, nullptr, 1, retPtrs))
        {
            LogError("[FeatureLua] WASM missing GetScriptBuffer export");
            return false;
        }
    }

    // Bounds-check.
    uint32_t memSize = 0;
    uint8_t* mem = GetMemory(&memSize);
    if (!mem || static_cast<uint64_t>(wasmPtr) + code.size() > memSize)
    {
        LogError("[FeatureLua] RunString: code ({} bytes) exceeds WASM buffer", code.size());
        return false;
    }

    std::memcpy(mem + wasmPtr, code.data(), code.size());

    // Call RunString(i32 len) → i32.
    const int32_t len = static_cast<int32_t>(code.size());
    int32_t result = -1;
    {
        const void* argPtrs[] = { &len };
        const void* retPtrs[] = { &result };
        CallExport("RunString", 1, argPtrs, 1, retPtrs);
    }
    if (result != 0)
    {
        LogError("[FeatureLua] RunString returned {} — Lua error (see console)", result);
        return false;
    }
    return true;
}
