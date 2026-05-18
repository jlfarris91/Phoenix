#pragma once

#include "PhoenixSim/Reflection/TypeDescriptorBuilder.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

namespace Phoenix
{
    // ── ScriptOptional<T> ─────────────────────────────────────────────────────
    //
    // A two-field struct that crosses the WASM sret boundary.
    //
    //   HasValue  — non-zero means the value is present.
    //   Value     — the actual value (only valid when HasValue != 0).
    //
    // Functions returning ScriptOptional<T> are recognized by LuaGen via the
    // "ScriptOptional" metadata key on the TypeDescriptor.  LuaGen emits:
    //
    //   if (_sret[hasValueSlot]) { push Value } else { lua_pushnil }
    //
    // This maps cleanly to Lua's nil-or-value idiom:
    //
    //   local v = entity:TryGetBlackboardValue_int32(key)
    //   if v then ... end
    //
    // No manual registration file is needed.  ScriptModuleBuilder calls
    // EnsureScriptOptionalRegistered<T>() automatically whenever a TemplateMethod
    // or TemplateFunction factory is found to return ScriptOptional<T>.

    template<class T>
    struct ScriptOptional
    {
        int32 HasValue = 0;
        T     Value    = {};

        static ScriptOptional None()           { return {};       }
        static ScriptOptional Some(const T& v) { return { 1, v }; }
    };

    // ── IsScriptOptional<T> ───────────────────────────────────────────────────
    //
    // Type trait that detects ScriptOptional<T> and exposes the inner type.

    template<class T>
    struct IsScriptOptional : std::false_type {};

    template<class TInner>
    struct IsScriptOptional<ScriptOptional<TInner>> : std::true_type
    {
        using Inner = TInner;
    };

    // ── EnsureScriptOptionalRegistered<TInner> ────────────────────────────────
    //
    // Programmatically registers ScriptOptional<TInner> with its two fields.
    //
    // Called automatically by ScriptModuleBuilder::RegisterTemplateVariant when
    // the factory return type is detected as ScriptOptional<TInner>.

    template<class TInner>
    void EnsureScriptOptionalRegistered(const std::string& innerSuffix)
    {
        TypeDescriptor& desc = TypeRegistry::Get<ScriptOptional<TInner>>();
        if (!desc.GetFields().empty())
            return;  // already registered

        TypeDescriptorBuilder<ScriptOptional<TInner>> b;
        b.Alias("ScriptOptional_" + innerSuffix)
         .Field("HasValue", &ScriptOptional<TInner>::HasValue)
         .Field("Value",    &ScriptOptional<TInner>::Value);
    }
}
