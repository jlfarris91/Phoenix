#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <memory>
#include <string>

#include "PhoenixSim/Scripting/IScriptRuntime.h"

namespace Phoenix
{
    class Session;

    // ── LuaRuntime ────────────────────────────────────────────────────────────
    //
    // sol2 adapter implementing IScriptRuntime.
    //
    // Owns no sol::state — it operates on the sol::state stored in
    // FeatureLuaDynamicBlock, passed in at construction.
    //
    // World functions use HasSelfParam=true on GenericFunction with self=World*.
    // LuaRuntime injects m_currentWorld as 'self' and reads only Params from Lua.

    class LuaRuntime : public IScriptRuntime
    {
    public:
        // session is borrowed — must outlive LuaRuntime.
        LuaRuntime(sol::state& state, Session* session);

        // ── IScriptRuntime ────────────────────────────────────────────────────

        void RegisterType(const TypeDescriptor& desc) override;

        void OpenNamespace(const char* dotSeparatedPath) override;
        void CloseNamespace() override;
        void RegisterFunction(const MethodDescriptor& fn) override;

        bool LoadFile(const char* path) override;
        bool ExecString(const std::string& code) override;

        void SetCurrentWorld(World* world) override;
        World* GetCurrentWorld() const { return m_currentWorld; }

        void OnBindingsComplete() override;

    private:
        // Splits a dot-separated path ("Phoenix.Unit") and returns (or creates)
        // the innermost table in the sol::state globals.
        sol::table GetOrCreateTable(const std::string& dotPath);

        // Registers a single MethodDescriptor into the given table.
        void RegisterFunctionInTable(sol::table& tbl, const MethodDescriptor& fn);

        // Convert a sol::object to GenericValue given the expected parameter type.
        static GenericValue SolToGenericValue(const sol::object& obj, const GenericValueTypeRef& type);

        // Convert a GenericValue to a sol::object for return to Lua.
        sol::object GenericValueToSol(const GenericValue& val) const;

        sol::state& m_state;
        Session*    m_session      = nullptr;
        World*      m_currentWorld = nullptr;

        // Stack of open namespace tables (for OpenNamespace/CloseNamespace).
        std::vector<sol::table> m_namespaceStack;
    };
}
