#pragma once

#include <string>
#include <vector>

#include "Phoenix/Platform.h"
#include "Phoenix/Reflection/MethodDescriptor.h"

namespace Phoenix
{
    class Session;

    // ── WasmHostEntry ─────────────────────────────────────────────────────────
    //
    // A C++ function registered as a WASM host import.
    // ImportModule is the WASM import module string (e.g. "Phoenix.Unit").
    // Descriptor carries the name, parameter types, and the generic invoker.

    struct PHOENIX_SIM_API WasmHostEntry
    {
        std::string ImportModule;
        MethodDescriptor Method;
    };

    class PHOENIX_SIM_API WasmRuntime
    {
    public:

        WasmRuntime(const std::shared_ptr<Session>& session);

        const std::string& GetScriptPath() const;
        const std::vector<uint8>& GetWasmBytes() const;
        const std::vector<WasmHostEntry>& GetRegistrations() const;

        bool LoadFile(const char* path);

    private:

        void RegisterType(const TypeDescriptor& desc);

        std::shared_ptr<Session>    Session;
        std::string                 ScriptPath;
        std::vector<uint8_t>        WasmBytes;
        std::vector<WasmHostEntry>  Registrations;
    };
}
