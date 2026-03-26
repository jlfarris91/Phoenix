#pragma once

#include <string>
#include <vector>

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Scripting/IScriptRuntime.h"

namespace Phoenix
{
    // ── WasmHostEntry ─────────────────────────────────────────────────────────
    //
    // A C++ function registered as a WASM host import.
    // ImportModule is the WASM import module string (e.g. "Phoenix.Unit").
    // Descriptor carries the name, parameter types, and the generic invoker.

    struct WasmHostEntry
    {
        std::string      ImportModule;
        MethodDescriptor Descriptor;
    };

    class WasmRuntime : public IScriptRuntime
    {
    public:

        WasmRuntime(const std::shared_ptr<Session>& session);

        const std::string& GetScriptPath() const { return ScriptPath; }
        const std::vector<uint8>& GetWasmBytes() const { return WasmBytes; }
        const std::vector<WasmHostEntry>& GetRegistrations() const { return Registrations; }

        // Begin IScriptRuntime implementation
        void RegisterType(const TypeDescriptor& desc) override;
        void OpenNamespace(const char* ns) override;
        void CloseNamespace() override;
        void RegisterFunction(const MethodDescriptor& fn) override;
        bool LoadFile(const char* path) override;
        bool ExecString(const std::string& /*code*/) override;
        void SetCurrentWorld(World* /*world*/) override;
        void OnBindingsComplete() override;
        // End IScriptRuntime implementation

    private:

        std::shared_ptr<Session>    Session;
        std::string                 ScriptPath;
        std::vector<uint8_t>        WasmBytes;
        std::vector<WasmHostEntry>  Registrations;
        std::string                 CurrentNamespace;
    };
}
