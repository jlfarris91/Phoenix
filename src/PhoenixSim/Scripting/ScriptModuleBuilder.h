#pragma once

#include <string>
#include <vector>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Reflection/MethodDescriptor.h"
#include "PhoenixSim/Reflection/MethodDescriptorString.h"
#include "PhoenixSim/Reflection/TypeDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    // ── ScriptFunctionEntry ───────────────────────────────────────────────────
    //
    // A single function registered into a dot-separated namespace.
    // WasmGen emits it as a flat host import under that namespace.
    // LuaGen emits it as a table entry.

    struct ScriptFunctionEntry
    {
        std::string      Namespace;   // e.g. "Phoenix" or "Phoenix.Unit"
        MethodDescriptor Method;
    };

    // ── ScriptClassEntry ──────────────────────────────────────────────────────
    //
    // An OOP class wrapping a handle type (e.g. UnitId).
    //
    // WasmGen flattens Methods and Statics into the Namespace as regular host
    // imports — the handle type is treated like any other WASM argument.
    //
    // LuaGen generates a metatable for HandleTypeId so that handle values
    // support method-call syntax (unit:IsAlive()). Statics are placed as
    // plain table entries (Unit.Spawn(...)).

    struct ScriptClassEntry
    {
        std::string              Namespace;     // Lua table path, e.g. "Phoenix.Unit"
        std::string              BaseNamespace; // Optional base class path, e.g. "Phoenix.Entity" (empty = no base)
        FName                    HandleTypeId;  // TypeId of the wrapped handle (e.g. UnitId)
        std::vector<MethodDescriptor> Methods;  // instance methods — handle injected as first non-World arg
        std::vector<MethodDescriptor> Statics;  // static methods — no handle injection
    };

    // ── ScriptNamespaceBuilder ────────────────────────────────────────────────
    //
    // Fluent builder returned by ScriptModuleBuilder::Namespace().
    // Registers free functions under a dot-separated namespace.

    class ScriptModuleBuilder;

    class ScriptNamespaceBuilder
    {
    public:
        ScriptNamespaceBuilder(ScriptModuleBuilder& module, std::string ns)
            : Module(module)
            , Ns(std::move(ns))
        {
            
        }

        template <class TRet, class... TArgs>
        ScriptNamespaceBuilder& Function(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            ScriptFunctionEntry entry;
            entry.Namespace = Ns;
            entry.Method    = BuildMethod<TRet, TArgs...>(name, fn);
            SetFlagRef(entry.Method.Flags, EMemberDescriptorFlags::Static);
            Emit(std::move(entry));
            return *this;
        }

        // Return to parent builder for further chaining.
        ScriptModuleBuilder& End() const { return Module; }

    private:
        template <class TRet, class... TArgs, size_t... I>
        static MethodDescriptor FillParams(
            MethodDescriptor& d,
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            std::index_sequence<I...>)
        {
            d.Name       = std::string(decl.Name);
            d.Params     = { ParamDescriptor{ std::string(decl.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
            d.ReturnType = &TypeRegistry::Get<TRet>();
            return d;
        }

        template <class TRet, class... TArgs>
        static MethodDescriptor BuildMethod(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, decl, std::index_sequence_for<TArgs...>{});
            return d;
        }

        void Emit(ScriptFunctionEntry entry) const;

        ScriptModuleBuilder& Module;
        std::string          Ns;
    };

    // ── ScriptClassBuilder ────────────────────────────────────────────────────
    //
    // Fluent builder returned by ScriptModuleBuilder::Class<THandle>().
    // Registers instance methods (handle injected after World) and statics.

    template <class THandle>
    class ScriptClassBuilder
    {
    public:
        ScriptClassBuilder(ScriptModuleBuilder& module, std::string ns);

        // Instance method: first non-World parameter must be THandle.
        template <class TRet, class... TArgs>
        ScriptClassBuilder& Method(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, name, std::index_sequence_for<TArgs...>{});
            Entry->Methods.push_back(std::move(d));
            return *this;
        }

        // Static method: no handle parameter.
        template <class TRet, class... TArgs>
        ScriptClassBuilder& StaticMethod(
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& name,
            TRet(*fn)(TArgs...))
        {
            MethodDescriptor d;
            d.Function = MakeGenericFunction(fn);
            FillParams<TRet, TArgs...>(d, name, std::index_sequence_for<TArgs...>{});
            SetFlagRef(d.Flags, EMemberDescriptorFlags::Static);
            Entry->Statics.push_back(std::move(d));
            return *this;
        }

        // Declare that this class inherits from an existing class at the given Lua path.
        // LuaGen will emit `setmetatable(Phoenix.Unit, {__index = Phoenix.Entity})` so
        // instance methods from the base are reachable via the normal lookup chain.
        ScriptClassBuilder& Inherits(const char* baseNamespace)
        {
            Entry->BaseNamespace = baseNamespace;
            return *this;
        }

        // Return to parent builder for further chaining.
        ScriptModuleBuilder& End() const { return Module; }

    private:
        template <class TRet, class... TArgs, size_t... I>
        static void FillParams(
            MethodDescriptor& d,
            const std::type_identity_t<MethodDeclarationString<TArgs...>>& decl,
            std::index_sequence<I...>)
        {
            d.Name       = std::string(decl.Name);
            d.Params     = { ParamDescriptor{ std::string(decl.ParamNames[I]), &TypeRegistry::Get<TArgs>() }... };
            d.ReturnType = &TypeRegistry::Get<TRet>();
        }

        ScriptModuleBuilder& Module;
        ScriptClassEntry*    Entry;
    };

    // ── ScriptModuleBuilder ───────────────────────────────────────────────────
    //
    // Top-level builder passed to IScriptBindings::Describe().
    // Accumulates ScriptFunctionEntry and ScriptClassEntry collections that
    // WasmGen and LuaGen query to emit their respective outputs.

    class PHOENIX_SIM_API ScriptModuleBuilder
    {
    public:
        // Open a dot-separated namespace for free function registration.
        ScriptNamespaceBuilder Namespace(const char* ns)
        {
            return ScriptNamespaceBuilder(*this, ns);
        }

        // Open an OOP class wrapping THandle under the given Lua namespace.
        template <class THandle>
        ScriptClassBuilder<THandle> Class(const char* ns)
        {
            return ScriptClassBuilder<THandle>(*this, ns);
        }

        void AddFunction(ScriptFunctionEntry entry)
        {
            Functions.push_back(std::move(entry));
        }

        void AddClass(ScriptClassEntry entry)
        {
            Classes.push_back(std::move(entry));
        }

        ScriptClassEntry* FindClass(const std::string& ns)
        {
            for (ScriptClassEntry& entry : Classes)
            {
                if (entry.Namespace == ns)
                    return &entry;
            }
            return nullptr;
        }

        const std::vector<ScriptFunctionEntry>& GetFunctions() const { return Functions; }
        const std::vector<ScriptClassEntry>&    GetClasses()   const { return Classes; }

    private:

        template <class T>
        friend class ScriptClassBuilder;

        std::vector<ScriptFunctionEntry> Functions;
        std::vector<ScriptClassEntry>    Classes;
    };

    // ── Out-of-line definitions ───────────────────────────────────────────────

    inline void ScriptNamespaceBuilder::Emit(ScriptFunctionEntry entry) const
    {
        Module.AddFunction(std::move(entry));
    }

    template <class THandle>
    ScriptClassBuilder<THandle>::ScriptClassBuilder(ScriptModuleBuilder& module, std::string ns)
        : Module(module)
    {
        Entry = Module.FindClass(ns);
        if (!Entry)
        {
            Entry = &Module.Classes.emplace_back(std::move(ns));
        }

        assert(FName::IsNoneOrEmpty(Entry->HandleTypeId) || Entry->HandleTypeId == StaticTypeName<THandle>::TypeId);
        Entry->HandleTypeId = StaticTypeName<THandle>::TypeId;
    }

} // namespace Phoenix
