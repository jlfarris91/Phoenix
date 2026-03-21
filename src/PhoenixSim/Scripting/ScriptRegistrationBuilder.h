#pragma once

#include "PhoenixSim/Reflection/Reflection.h"

namespace Phoenix
{
    // ── ScriptRegistrationBuilder<T> ─────────────────────────────────────────
    //
    // Fluent builder used inside PHX_SCRIPT_REGISTRATION blocks.
    // Writes MethodDescriptors (with ScriptNamespace set) into the type's
    // TypeDescriptor::Methods map via TypeRegistry, sharing the same map as
    // reflection-registered methods.
    //
    // LuaRuntime filters by non-empty MethodDescriptor::ScriptNamespace when
    // binding to Lua tables, so reflection-only methods are never exposed.
    //
    // Usage:
    //
    //   PHX_SCRIPT_REGISTRATION(FeatureUnit)
    //   {
    //       registration
    //           .namespace_("Phoenix.Unit")
    //           .world_function("SpawnUnit", &Script_Unit_Spawn);
    //   }

    template <class T>
    class ScriptRegistrationBuilder
    {
    public:
        explicit ScriptRegistrationBuilder()
            : m_desc(&TypeRegistry::GetOrCreate<T>())
        {}

        // ── Namespace ─────────────────────────────────────────────────────────

        ScriptRegistrationBuilder& namespace_(const char* ns)
        {
            m_currentNamespace = ns;
            return *this;
        }

        // ── Functions ─────────────────────────────────────────────────────────
        //
        // Both function() and world_function() call MakeMethodDescriptor —
        // there is no semantic distinction between them.  WorldRef is a normal
        // registered struct param; LuaRuntime detects it by TypeDescriptor
        // pointer comparison and injects the current world automatically.

        template <class TRet, class... TArgs>
        ScriptRegistrationBuilder& function(const char* name, TRet(*fn)(TArgs...))
        {
            MethodDescriptor d = MakeMethodDescriptor(name, fn);
            d.ScriptNamespace  = m_currentNamespace;
            m_desc->Methods.emplace(name, std::move(d));
            return *this;
        }

        template <class TRet, class... TArgs>
        ScriptRegistrationBuilder& world_function(const char* name, TRet(*fn)(TArgs...))
        {
            MethodDescriptor d = MakeMethodDescriptor(name, fn);
            d.ScriptNamespace  = m_currentNamespace;
            m_desc->Methods.emplace(name, std::move(d));
            return *this;
        }

    private:
        TypeDescriptor* m_desc             = nullptr;
        std::string     m_currentNamespace;
    };

    // ── PHX_SCRIPT_REGISTRATION ───────────────────────────────────────────────
    //
    // Places registration code in a static initializer that runs before main().
    // The body has access to a pre-bound 'registration' ScriptRegistrationBuilder<Type>.
    //
    // Can appear multiple times with different types in the same .cpp file.

#define PHX_SCRIPT_REGISTRATION(Type)                                                    \
    static void _PhxScriptReg_##Type##_body(                                             \
        Phoenix::ScriptRegistrationBuilder<Type>& registration);                         \
    namespace {                                                                          \
        struct _PhxScriptReg_##Type##_t                                                  \
        {                                                                                \
            _PhxScriptReg_##Type##_t() noexcept                                          \
            {                                                                            \
                Phoenix::ScriptRegistrationBuilder<Type> registration;                   \
                _PhxScriptReg_##Type##_body(registration);                               \
            }                                                                            \
        } _phx_script_reg_##Type##_instance;                                             \
    }                                                                                    \
    static void _PhxScriptReg_##Type##_body(                                             \
        Phoenix::ScriptRegistrationBuilder<Type>& registration)

} // namespace Phoenix
