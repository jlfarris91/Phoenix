/*
 * PhoenixWasmGen.cpp — WASM host-API and Lua bridge code generator.
 *
 * Usage:
 *   PhoenixWasmGen --host-api  <out/host_api.h>
 *                  --lua-bridge <out/lua_bridge.c>
 *
 * Outputs:
 *   host_api.h   — Language-agnostic C header with one
 *                  __attribute__((import_module/import_name)) extern declaration
 *                  per script-visible static method.  Any language compiled to
 *                  WASM (C, Rust, AssemblyScript, …) can include this header to
 *                  call Phoenix host functions.
 *
 *   lua_bridge.c — Lua-specific marshaling: one static l_* function per method
 *                  that unpacks Lua stack args, calls the corresponding import,
 *                  and pushes the result.  Also emits register_phoenix_api()
 *                  which builds the nested namespace tables in the Lua state.
 *
 * How registrations are discovered:
 *   CMakeLists.txt links PhoenixWasmGen with /WHOLE_ARCHIVE for every library,
 *   which forces the linker to include ALL object files regardless of symbol
 *   references.  Every PHX_DEFINE_TYPE static initializer therefore runs at
 *   startup and populates the TypeRegistry automatically.
 *
 * Adding a new module:
 *   Add a Namespace() + StaticMethod() registration to the type's PHX_DEFINE_TYPE
 *   block.  No changes to this file are needed.
 */

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "PhoenixSim/Reflection/Reflection.h"
#include "PhoenixSim/Reflection/GenericValue.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;

// ── WASM type helpers ─────────────────────────────────────────────────────────
// Must match WasmEnvironment::ToWasmTypeChar exactly.

static char WasmTypeChar(const GenericValueTypeRef& type)
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
        return 'i';  // TFixed stores int32_t internally
    case EGenericValueType::Double:
        return 'F';
    default:
        return 'v';
    }
}

static const char* CType(char w)
{
    switch (w) {
    case 'i': return "int32_t";
    case 'I': return "int64_t";
    case 'f': return "float";
    case 'F': return "double";
    default:  return nullptr;
    }
}

// "Phoenix.Unit" + "SpawnUnit" → "phx_Phoenix_Unit_SpawnUnit"
// Any character that is not [A-Za-z0-9_] is replaced with '_' so the result
// is always a valid C identifier (important for template type namespaces).
static std::string ImportFuncName(const std::string& ns, const std::string& method)
{
    auto sanitize = [](const std::string& s, std::string& out) {
        for (char c : s)
            out += (std::isalnum(static_cast<unsigned char>(c)) || c == '_') ? c : '_';
    };
    std::string r = "phx_";
    sanitize(ns, r);
    r += '_';
    sanitize(method, r);
    return r;
}

// Lua C wrapper name: "_phx_l_Phoenix_Unit_SpawnUnit"
static std::string LuaFuncName(const std::string& ns, const std::string& method)
{
    return "_phx_l_" + ImportFuncName(ns, method).substr(4); // strip leading "phx_"
}

static std::vector<std::string> SplitDot(const std::string& s)
{
    std::vector<std::string> parts;
    size_t start = 0;
    for (size_t i = 0; i <= s.size(); ++i)
    {
        if (i == s.size() || s[i] == '.')
        {
            parts.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    return parts;
}

// ── Struct marshaling utilities ───────────────────────────────────────────────

// Returns properties of desc sorted alphabetically (deterministic field order).
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

static bool IsExpandableStruct(const GenericValueTypeRef& type)
{
    return type.IsStruct() && type.Descriptor &&
           type.Descriptor->GetSize() <= 32;
}

// Flat description of one scalar field in a struct (nested structs are expanded).
struct StructField
{
    std::string Name;           // e.g. "X"
    char        WasmChar;       // 'i','I','f','F'
    size_t      ByteOffset;     // byte offset within the struct's sret buffer
    int         FractionalBits; // > 0 for FixedPoint fields; 0 for plain scalars
};

// Returns a flat sorted list of all scalar fields (recursively expanding nested structs).
// ByteOffset is set to sequential positions (4 bytes per i/f slot, 8 per I/F slot).
// Nested struct field names are prefixed: e.g. Position.X becomes "Position_X".
static std::vector<StructField> FlattenStructFields(const TypeDescriptor& desc,
                                                     const std::string& prefix = "")
{
    std::vector<StructField> result;
    size_t offset = 0;
    for (const auto& [name, prop] : GetSortedProps(desc))
    {
        const std::string qualName = prefix.empty() ? name : prefix + "_" + name;
        GenericValueTypeRef tr = prop->GetTypeRef();
        if (IsExpandableStruct(tr))
        {
            // Nested struct: expand recursively with qualified prefix.
            auto nested = FlattenStructFields(*tr.Descriptor, qualName);
            for (auto& f : nested)
            {
                f.ByteOffset += offset;
                result.push_back(f);
            }
            offset += tr.Descriptor->GetSize();
        }
        else
        {
            char c = WasmTypeChar(tr);
            if (c == 'v') continue;
            const size_t sz = (c == 'I' || c == 'F') ? 8 : 4;
            // Read FractionalBits from PropertyDescriptor metadata (for FixedPoint fields)
            // or from the type's TypeDescriptor (stored via GenericValueTypeRefMaker<TFixed>).
            int fracBits = 0;
            if (tr.Primitive == EGenericValueType::FixedPoint)
            {
                // Check PropertyDescriptor::Metadata first (populated by TypeDescriptorMetadataProvider)
                auto it = prop->Metadata.find("FractionalBits");
                if (it != prop->Metadata.end())
                    fracBits = std::stoi(it->second);
                // Fallback: check the type's own TypeDescriptor (stored via GenericValueTypeRefMaker)
                else if (tr.Descriptor)
                {
                    auto it2 = tr.Descriptor->GetMetadata().find("FractionalBits");
                    if (it2 != tr.Descriptor->GetMetadata().end())
                        fracBits = std::stoi(it2->second);
                }
            }
            result.push_back({ qualName, c, offset, fracBits });
            offset += sz;
        }
    }
    return result;
}

// Convert a C++ qualified name ("Phoenix::Vec2") to a Lua path ("Phoenix.Vec2").
static std::string QualNameToLuaPath(const std::string& qualName)
{
    std::string s = qualName;
    for (size_t i = 0; i + 1 < s.size(); )
    {
        if (s[i] == ':' && s[i + 1] == ':') { s.replace(i, 2, "."); }
        else ++i;
    }
    return s;
}

struct StructInfo
{
    std::string              CName;    // "Vec2"
    std::string              LuaPath;  // "Phoenix.Vec2"
    std::vector<StructField> Fields;   // flat sorted scalar fields
    const TypeDescriptor*    Desc;
};

static std::vector<StructInfo> CollectStructTypes(const TypeDescriptor* worldDesc)
{
    std::vector<StructInfo> structs;
    for (const auto& [_, desc] : TypeRegistry::GetAll())
    {
        if (!desc) continue;
        if (desc.get() == worldDesc) continue;
        if (desc->IsNoScriptTable()) continue;
        if (desc->GetProperties().empty()) continue;
        if (desc->GetSize() > 32) continue;

        auto fields = FlattenStructFields(*desc);
        if (fields.empty()) continue;

        StructInfo si;
        si.CName   = desc->GetCName();
        si.LuaPath = QualNameToLuaPath(desc->GetQualifiedCName());
        si.Fields  = std::move(fields);
        si.Desc    = desc.get();
        structs.push_back(std::move(si));
    }
    std::sort(structs.begin(), structs.end(),
        [](const StructInfo& a, const StructInfo& b) { return a.CName < b.CName; });
    return structs;
}

// Emit code to place the table at Lua stack top into the path (e.g. "Phoenix.Vec2").
// Consumes the table from the stack.
static void EmitPlaceAtLuaPath(FILE* out, const std::string& luaPath, int indent)
{
    std::string pad(indent * 4, ' ');
    auto parts = SplitDot(luaPath);

    if (parts.size() == 1)
    {
        fprintf(out, "%slua_setglobal(L, \"%s\");\n", pad.c_str(), parts[0].c_str());
        return;
    }

    // Stack: ..., type_table
    fprintf(out, "%slua_getglobal(L, \"%s\");\n", pad.c_str(), parts[0].c_str());
    fprintf(out, "%sif (lua_isnil(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
                 "lua_pushvalue(L, -1); lua_setglobal(L, \"%s\"); }\n",
            pad.c_str(), parts[0].c_str());
    // Stack: ..., type_table, root_table

    for (size_t i = 1; i + 1 < parts.size(); ++i)
    {
        fprintf(out, "%slua_getfield(L, -1, \"%s\");\n", pad.c_str(), parts[i].c_str());
        fprintf(out, "%sif (lua_isnil(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
                     "lua_pushvalue(L, -1); lua_setfield(L, -3, \"%s\"); }\n",
                pad.c_str(), parts[i].c_str());
        fprintf(out, "%slua_remove(L, -2);\n", pad.c_str());
        // Stack: ..., type_table, inner_table
    }

    // Stack: ..., type_table, parent_table
    fprintf(out, "%slua_insert(L, -2);\n", pad.c_str());
    // Stack: ..., parent_table, type_table
    fprintf(out, "%slua_setfield(L, -2, \"%s\");\n", pad.c_str(), parts.back().c_str());
    fprintf(out, "%slua_pop(L, 1);\n", pad.c_str());
    // Stack: ...
}

// ── struct helper code generation ────────────────────────────────────────────

static void GenerateStructHelpers(FILE* out, const std::vector<StructInfo>& structs)
{
    if (structs.empty()) return;

    fprintf(out, "\n/* ── Struct pack/unpack helpers ──────────────────────────────────── */\n\n");

    for (const auto& si : structs)
    {
        // _phx_unpack_<T>(lua_State* L, int idx, T0* f0, T1* f1, ...)
        fprintf(out, "static void _phx_unpack_%s(lua_State* L, int idx", si.CName.c_str());
        for (const auto& f : si.Fields)
            fprintf(out, ", %s* %s", CType(f.WasmChar), f.Name.c_str());
        fprintf(out, ")\n{\n");
        for (const auto& f : si.Fields)
        {
            const bool isFixed = (f.FractionalBits > 0);
            if (isFixed)
            {
                // FixedPoint: Lua passes a real-world number, scale to raw int32.
                fprintf(out,
                    "    lua_getfield(L, idx, \"%s\"); "
                    "*%s = (%s)(lua_tonumber(L, -1) * %d.0); lua_pop(L, 1);\n",
                    f.Name.c_str(), f.Name.c_str(), CType(f.WasmChar),
                    1 << f.FractionalBits);
            }
            else
            {
                const char* luaGet = (f.WasmChar == 'f' || f.WasmChar == 'F')
                                         ? "lua_tonumber"
                                         : "lua_tointeger";
                fprintf(out,
                    "    lua_getfield(L, idx, \"%s\"); *%s = (%s)%s(L, -1); lua_pop(L, 1);\n",
                    f.Name.c_str(), f.Name.c_str(), CType(f.WasmChar), luaGet);
            }
        }
        fprintf(out, "}\n\n");

        // _phx_pack_<T>(lua_State* L, T0 f0, T1 f1, ...)
        fprintf(out, "static void _phx_pack_%s(lua_State* L", si.CName.c_str());
        for (const auto& f : si.Fields)
            fprintf(out, ", %s %s", CType(f.WasmChar), f.Name.c_str());
        fprintf(out, ")\n{\n");
        fprintf(out, "    lua_newtable(L);\n");
        for (const auto& f : si.Fields)
        {
            if (f.FractionalBits > 0)
            {
                // FixedPoint: convert raw int32 back to real-world number.
                fprintf(out,
                    "    lua_pushnumber(L, (lua_Number)%s / %d.0); lua_setfield(L, -2, \"%s\");\n",
                    f.Name.c_str(), 1 << f.FractionalBits, f.Name.c_str());
            }
            else
            {
                const char* luaPush = (f.WasmChar == 'f' || f.WasmChar == 'F')
                                          ? "lua_pushnumber"
                                          : "lua_pushinteger";
                fprintf(out,
                    "    %s(L, (%s)%s); lua_setfield(L, -2, \"%s\");\n",
                    luaPush,
                    (f.WasmChar == 'f' || f.WasmChar == 'F') ? "lua_Number" : "lua_Integer",
                    f.Name.c_str(), f.Name.c_str());
            }
        }
        fprintf(out,
            "    lua_getfield(L, LUA_REGISTRYINDEX, \"_phx_mt_%s\");\n"
            "    if (lua_istable(L, -1)) lua_setmetatable(L, -2);\n"
            "    else lua_pop(L, 1);\n"
            "}\n\n",
            si.CName.c_str());

        // __tostring metamethod
        fprintf(out, "static int _phx_%s_tostring(lua_State* L)\n{\n", si.CName.c_str());
        for (const auto& f : si.Fields)
            fprintf(out, "    %s %s;\n", CType(f.WasmChar), f.Name.c_str());
        fprintf(out, "    _phx_unpack_%s(L, 1", si.CName.c_str());
        for (const auto& f : si.Fields)
            fprintf(out, ", &%s", f.Name.c_str());
        fprintf(out, ");\n");
        // Build the format string (FixedPoint fields display as scaled real-world numbers)
        fprintf(out, "    lua_pushfstring(L, \"%s{", si.CName.c_str());
        for (size_t i = 0; i < si.Fields.size(); ++i)
        {
            if (i) fprintf(out, ", ");
            const bool isNum = (si.Fields[i].WasmChar == 'f' || si.Fields[i].WasmChar == 'F'
                                || si.Fields[i].FractionalBits > 0);
            fprintf(out, "%s=%s", si.Fields[i].Name.c_str(), isNum ? "%%g" : "%%d");
        }
        fprintf(out, "}\"");
        for (const auto& f : si.Fields)
        {
            if (f.FractionalBits > 0)
                fprintf(out, ", (double)%s / %d.0", f.Name.c_str(), 1 << f.FractionalBits);
            else if (f.WasmChar == 'f' || f.WasmChar == 'F')
                fprintf(out, ", (double)%s", f.Name.c_str());
            else if (f.WasmChar == 'I')
                fprintf(out, ", (long long)%s", f.Name.c_str());
            else
                fprintf(out, ", (int)%s", f.Name.c_str());
        }
        fprintf(out, ");\n    return 1;\n}\n\n");
    }
}

// Emit struct type table + metatable registration inside register_phoenix_api.
static void EmitStructTypeTableRegistration(FILE* out, const std::vector<StructInfo>& structs)
{
    if (structs.empty()) return;
    fprintf(out, "\n    /* ── Struct type tables ── */\n");
    for (const auto& si : structs)
    {
        fprintf(out, "\n    /* %s */\n", si.CName.c_str());
        fprintf(out, "    lua_newtable(L);\n");
        fprintf(out, "    lua_pushvalue(L, -1); lua_setfield(L, -2, \"__index\");\n");
        fprintf(out, "    lua_pushstring(L, \"%s\"); lua_setfield(L, -2, \"__type\");\n",
                si.CName.c_str());
        fprintf(out,
            "    lua_pushcfunction(L, _phx_%s_tostring); lua_setfield(L, -2, \"__tostring\");\n",
            si.CName.c_str());
        // Non-static instance methods (if any)
        std::vector<std::pair<std::string, const MethodDescriptor*>> methods;
        for (const auto& [n, m] : si.Desc->GetMethods())
            if (!m.IsStatic() && !HasAnyFlags(m.Flags, EMemberDescriptorFlags::ScriptHidden))
                methods.push_back({ n, &m });
        std::sort(methods.begin(), methods.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        // (Instance methods are not bridged through WASM imports but can be added manually.)
        // Store in registry for _phx_pack_ lookup
        fprintf(out,
            "    lua_pushvalue(L, -1);\n"
            "    lua_setfield(L, LUA_REGISTRYINDEX, \"_phx_mt_%s\");\n",
            si.CName.c_str());
        // Place at user-visible Lua path (e.g. Phoenix.Vec2)
        EmitPlaceAtLuaPath(out, si.LuaPath, 1);
    }
}

// ── Collected method info ─────────────────────────────────────────────────────

// Returns the script-visible namespace for a type: explicit Namespace() metadata
// takes priority; otherwise the qualified C++ name is converted to a Lua path.
static std::string NamespaceOfDesc(const TypeDescriptor& desc)
{
    const auto it = desc.GetMetadata().find("Namespace");
    if (it != desc.GetMetadata().end()) return it->second;
    // QualifiedTypeCName<T> is a compile-time template; at runtime we use the
    // descriptor's own qualified name getter.
    return QualNameToLuaPath(desc.GetQualifiedCName());
}

struct MethodInfo
{
    std::string              Namespace;
    std::string              Name;
    const MethodDescriptor*  Desc      = nullptr;  // points into TypeDescriptor OR OwnedDesc
    std::optional<MethodDescriptor> OwnedDesc;     // storage for synthesized accessors

    // Safe accessor — always valid after construction.
    const MethodDescriptor& GetDesc() const { return OwnedDesc ? *OwnedDesc : *Desc; }

    // Copy / move — keep Desc in sync with OwnedDesc.
    MethodInfo() = default;
    MethodInfo(const MethodInfo& o)
        : Namespace(o.Namespace), Name(o.Name), OwnedDesc(o.OwnedDesc)
    { Desc = OwnedDesc ? &*OwnedDesc : o.Desc; }
    MethodInfo(MethodInfo&& o) noexcept
        : Namespace(std::move(o.Namespace)), Name(std::move(o.Name))
        , Desc(o.Desc), OwnedDesc(std::move(o.OwnedDesc))
    { if (OwnedDesc) Desc = &*OwnedDesc; }
    MethodInfo& operator=(const MethodInfo& o)
    {
        Namespace = o.Namespace; Name = o.Name; OwnedDesc = o.OwnedDesc;
        Desc = OwnedDesc ? &*OwnedDesc : o.Desc;
        return *this;
    }
    MethodInfo& operator=(MethodInfo&& o) noexcept
    {
        Namespace = std::move(o.Namespace); Name = std::move(o.Name);
        OwnedDesc = std::move(o.OwnedDesc); Desc = o.Desc;
        if (OwnedDesc) Desc = &*OwnedDesc;
        return *this;
    }
};

static std::vector<MethodInfo> CollectMethods(const TypeDescriptor* worldDesc)
{
    // Collect all types that have something script-visible.
    std::vector<const TypeDescriptor*> types;
    for (const auto& [_, desc] : TypeRegistry::GetAll())
    {
        if (!desc) continue;
        if (desc.get() == worldDesc) continue;
        if (desc->IsScriptHidden()) continue;

        bool hasStaticMethods = false;
        for (const auto& [n, m] : desc->GetMethods())
            if (m.IsStatic() && !HasAnyFlags(m.Flags, EMemberDescriptorFlags::ScriptHidden))
                { hasStaticMethods = true; break; }

        const bool isValueType = (desc->GetSize() > 0 && desc->GetSize() <= 32);
        const bool hasFields   = !desc->GetProperties().empty();

        if (!hasStaticMethods && !(isValueType && hasFields)) continue;
        types.push_back(desc.get());
    }

    std::sort(types.begin(), types.end(), [](const TypeDescriptor* a, const TypeDescriptor* b)
    { return NamespaceOfDesc(*a) < NamespaceOfDesc(*b); });

    std::vector<MethodInfo> methods;
    for (const TypeDescriptor* desc : types)
    {
        const std::string ns = NamespaceOfDesc(*desc);

        // ── Static methods ────────────────────────────────────────────────────
        {
            std::vector<std::pair<std::string, const MethodDescriptor*>> sorted;
            for (const auto& [n, m] : desc->GetMethods())
            {
                if (!m.IsStatic()) continue;
                if (HasAnyFlags(m.Flags, EMemberDescriptorFlags::ScriptHidden)) continue;
                sorted.push_back({ n, &m });
            }
            std::sort(sorted.begin(), sorted.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });
            for (const auto& [n, m] : sorted)
            {
                MethodInfo mi;
                mi.Namespace = ns;
                mi.Name      = n;
                mi.Desc      = m;
                methods.push_back(std::move(mi));
            }
        }

        // ── Field getter / setter for value types (≤32 bytes) ─────────────────
        // These are synthesized as static functions:
        //   get_<field>(self: T) -> fieldType
        //   set_<field>(self: T, value: fieldType) -> T  (returns modified copy)
        if (desc->GetSize() > 0 && desc->GetSize() <= 32 && !desc->GetProperties().empty())
        {
            const GenericValueTypeRef selfType { EGenericValueType::Unknown, desc };

            std::vector<std::pair<std::string, const PropertyDescriptor*>> sortedProps;
            for (const auto& [n, p] : desc->GetProperties())
            {
                if (HasAnyFlags(p.Flags, EMemberDescriptorFlags::ScriptHidden)) continue;
                sortedProps.push_back({ n, &p });
            }
            std::sort(sortedProps.begin(), sortedProps.end(),
                [](const auto& a, const auto& b) { return a.first < b.first; });

            for (const auto& [propName, prop] : sortedProps)
            {
                const GenericValueTypeRef fieldType = prop->GetTypeRef();
                // Skip field types we can't express in the WASM ABI.
                if (WasmTypeChar(fieldType) == 'v' && !IsExpandableStruct(fieldType)) continue;

                // Getter: get_<field>(self) -> field
                {
                    MethodInfo mi;
                    mi.Namespace = ns;
                    mi.Name      = "get_" + propName;
                    mi.OwnedDesc.emplace();
                    mi.OwnedDesc->Name = mi.Name;
                    mi.OwnedDesc->Params.push_back({ "self", selfType });
                    mi.OwnedDesc->Return.Type = fieldType;
                    mi.Desc = &*mi.OwnedDesc;
                    methods.push_back(std::move(mi));
                }

                // Setter: set_<field>(self, value) -> T
                {
                    MethodInfo mi;
                    mi.Namespace = ns;
                    mi.Name      = "set_" + propName;
                    mi.OwnedDesc.emplace();
                    mi.OwnedDesc->Name = mi.Name;
                    mi.OwnedDesc->Params.push_back({ "self",  selfType  });
                    mi.OwnedDesc->Params.push_back({ "value", fieldType });
                    mi.OwnedDesc->Return.Type = selfType;  // returns modified struct
                    mi.Desc = &*mi.OwnedDesc;
                    methods.push_back(std::move(mi));
                }
            }
        }
    }
    return methods;
}

// ── host_api.h generation ─────────────────────────────────────────────────────

static void GenerateHostApi(FILE* out, const std::vector<MethodInfo>& methods,
                             const TypeDescriptor* worldDesc)
{
    fprintf(out,
        "/* Auto-generated by PhoenixWasmGen — do not edit.\n"
        " * Regenerate: build the PhoenixWasmGen target.\n"
        " *\n"
        " * Calling convention (matches WasmEnvironment::BuildWasmSignature):\n"
        " *   • WorldRef params are injected by the host — NOT in the WASM signature.\n"
        " *   • Struct params (≤32 bytes) are expanded to scalar fields (alphabetical order).\n"
        " *   • Struct returns use sret convention: void return + i32 sret as first param.\n"
        " *   • i32 → int32_t,  i64 → int64_t,  f32 → float,  f64 → double\n"
        " */\n\n"
        "#pragma once\n"
        "#include <stdint.h>\n\n"
    );

    std::string lastNs;
    for (const auto& mi : methods)
    {
        if (mi.Namespace != lastNs)
        {
            if (!lastNs.empty()) fprintf(out, "\n");
            fprintf(out, "/* ── %s ── */\n\n", mi.Namespace.c_str());
            lastNs = mi.Namespace;
        }

        // Build WASM param list (structs expanded, struct return → sret).
        const bool retIsStruct = IsExpandableStruct(mi.GetDesc().Return.Type);
        std::vector<char> paramChars;
        std::string notes;

        if (retIsStruct) paramChars.push_back('i');  // sret pointer

        for (const auto& p : mi.GetDesc().Params)
        {
            if (p.Type.Descriptor == worldDesc) { notes += " World"; continue; }
            if (IsExpandableStruct(p.Type))
            {
                for (const auto& f : FlattenStructFields(*p.Type.Descriptor))
                    paramChars.push_back(f.WasmChar);
                notes += " Struct";
            }
            else
            {
                char c = WasmTypeChar(p.Type);
                if (c != 'v') paramChars.push_back(c);
                else          notes += " Unknown";
            }
        }

        char retChar = retIsStruct ? 'v' : WasmTypeChar(mi.GetDesc().Return.Type);
        const char* retC = (retChar == 'v' || !CType(retChar)) ? "void" : CType(retChar);

        std::string params;
        for (size_t i = 0; i < paramChars.size(); ++i)
        {
            if (i) params += ", ";
            params += CType(paramChars[i]);
            params += " p"; params += std::to_string(i);
        }
        if (params.empty()) params = "void";

        std::string sig;
        sig += retChar; sig += '(';
        for (char c : paramChars) sig += c;
        sig += ')';

        if (!notes.empty())
            fprintf(out, "/* %s: %s  [injected:%s] */\n", mi.Name.c_str(), sig.c_str(), notes.c_str());
        else
            fprintf(out, "/* %s: %s */\n", mi.Name.c_str(), sig.c_str());

        fprintf(out,
            "__attribute__((import_module(\"%s\"), import_name(\"%s\")))\n"
            "extern %s %s(%s);\n\n",
            mi.Namespace.c_str(), mi.Name.c_str(),
            retC, ImportFuncName(mi.Namespace, mi.Name).c_str(),
            params.c_str());
    }
}

// ── lua_bridge.c generation ───────────────────────────────────────────────────

// Namespace tree for building the nested Lua table registration.
struct NSNode
{
    // Methods whose type's namespace exactly equals this node's full path.
    std::vector<std::pair<std::string, std::string>> Methods; // {lua_name, c_func_name}
    // Child namespace components.
    std::map<std::string, NSNode> Children;
};

static void InsertIntoTree(NSNode& root, const std::vector<std::string>& parts,
                            const std::string& luaName, const std::string& cFunc, size_t depth = 0)
{
    if (depth + 1 == parts.size())
    {
        root.Methods.push_back({ luaName, cFunc });
        return;
    }
    InsertIntoTree(root.Children[parts[depth + 1]], parts, luaName, cFunc, depth + 1);
}

// Emit the recursive table-building code.
// indent: current indentation level (2 spaces each).
static void EmitTableRegistration(FILE* out, const NSNode& node, int indent)
{
    std::string pad(indent * 4, ' ');

    // Register methods at this level.
    for (const auto& [luaName, cFunc] : node.Methods)
    {
        fprintf(out, "%slua_pushcfunction(L, %s);\n", pad.c_str(), cFunc.c_str());
        fprintf(out, "%slua_setfield(L, -2, \"%s\");\n", pad.c_str(), luaName.c_str());
    }

    // Recurse into children (already sorted by std::map).
    for (const auto& [childName, child] : node.Children)
    {
        fprintf(out, "%slua_newtable(L);\n", pad.c_str());
        EmitTableRegistration(out, child, indent);
        fprintf(out, "%slua_setfield(L, -2, \"%s\");\n", pad.c_str(), childName.c_str());
    }
}

static void GenerateLuaBridge(FILE* out, const std::vector<MethodInfo>& methods,
                               const std::vector<StructInfo>& structs,
                               const TypeDescriptor* worldDesc)
{
    fprintf(out,
        "/* Auto-generated by PhoenixWasmGen — do not edit.\n"
        " * Regenerate: build the PhoenixWasmGen target.\n"
        " *\n"
        " * Provides:\n"
        " *   • One static l_* Lua C function per registered host method.\n"
        " *   • register_phoenix_api(lua_State*) — builds the nested Phoenix.*\n"
        " *     namespace tables in the Lua state.\n"
        " */\n\n"
        "#include \"host_api.h\"\n"
        "#include \"lua.h\"\n"
        "#include \"lauxlib.h\"\n"
        "#include <stdint.h>\n"
        "#include <string.h>\n\n"
    );

    // FNV1A-32 helper — used for FName params.
    fprintf(out,
        "static uint32_t _phx_fnv1a32(const char* s, size_t n)\n"
        "{\n"
        "    uint32_t h = 0x811c9dc5u;\n"
        "    while (n--) { h ^= (uint8_t)*s++; h *= 0x01000193u; }\n"
        "    return h;\n"
        "}\n\n"
    );

    // Struct helpers (forward-declared before method wrappers that use them).
    GenerateStructHelpers(out, structs);

    // Build a lookup from CName → StructInfo for quick reference.
    std::map<std::string, const StructInfo*> structByCName;
    for (const auto& si : structs) structByCName[si.CName] = &si;

    // Emit one Lua C wrapper per method.
    for (const auto& mi : methods)
    {
        fprintf(out, "/* %s.%s */\n", mi.Namespace.c_str(), mi.Name.c_str());
        fprintf(out, "static int %s(lua_State* L)\n{\n",
                LuaFuncName(mi.Namespace, mi.Name).c_str());

        // Describe each non-world param as a segment that:
        //   • declares local vars
        //   • contributes call argument expressions
        struct Segment {
            std::string Decl;                  // declaration / unpack code
            std::vector<std::string> CallArgs;  // expressions to pass to the import
        };
        std::vector<Segment> segments;
        int luaIdx = 1;
        int callIdx = 0;

        const bool retIsStruct = IsExpandableStruct(mi.GetDesc().Return.Type);

        for (const auto& p : mi.GetDesc().Params)
        {
            if (p.Type.Descriptor == worldDesc) continue;

            if (IsExpandableStruct(p.Type))
            {
                // Struct param: one Lua arg (a table), multiple WASM args.
                const char* cn = p.Type.Descriptor->GetCName();
                const StructInfo* si = structByCName.count(cn) ? structByCName[cn] : nullptr;
                if (!si) { ++luaIdx; continue; }

                Segment seg;
                // Declare a local var per field.
                for (const auto& f : si->Fields)
                {
                    seg.Decl += std::string("    ") + CType(f.WasmChar) +
                                " p" + std::to_string(callIdx) + "_" + f.Name + ";\n";
                }
                // Unpack call.
                seg.Decl += "    _phx_unpack_" + std::string(cn) +
                            "(L, " + std::to_string(luaIdx);
                for (const auto& f : si->Fields)
                    seg.Decl += ", &p" + std::to_string(callIdx) + "_" + f.Name;
                seg.Decl += ");\n";
                // Each field becomes a call arg.
                for (const auto& f : si->Fields)
                    seg.CallArgs.push_back("p" + std::to_string(callIdx) + "_" + f.Name);
                callIdx += (int)si->Fields.size();
                ++luaIdx;
                segments.push_back(std::move(seg));
                continue;
            }

            char c = WasmTypeChar(p.Type);
            if (c == 'v') { ++luaIdx; continue; }

            bool isFName    = (p.Type.Primitive == EGenericValueType::Name);
            bool isFixed    = (p.Type.Primitive == EGenericValueType::FixedPoint);
            std::string var = "p" + std::to_string(callIdx);
            Segment seg;
            if (isFName)
            {
                seg.Decl = "    const char* _s" + std::to_string(callIdx) +
                           " = luaL_checkstring(L, " + std::to_string(luaIdx) + ");\n";
                seg.CallArgs.push_back(
                    "(int32_t)_phx_fnv1a32(_s" + std::to_string(callIdx) +
                    ", strlen(_s" + std::to_string(callIdx) + "))");
            }
            else if (isFixed && p.Type.Descriptor)
            {
                // FixedPoint param with known FractionalBits — scale from real-world number.
                int fracBits = 0;
                auto it = p.Type.Descriptor->GetMetadata().find("FractionalBits");
                if (it != p.Type.Descriptor->GetMetadata().end())
                    fracBits = std::stoi(it->second);
                const char* ct = CType(c);
                if (fracBits > 0)
                    seg.Decl = "    " + std::string(ct) + " " + var +
                               " = (" + ct + ")(luaL_checknumber(L, " +
                               std::to_string(luaIdx) + ") * " +
                               std::to_string(1 << fracBits) + ".0);\n";
                else
                    seg.Decl = "    " + std::string(ct) + " " + var +
                               " = (" + ct + ")luaL_checkinteger(L, " +
                               std::to_string(luaIdx) + ");\n";
                seg.CallArgs.push_back(var);
            }
            else
            {
                const char* ct = CType(c);
                switch (c) {
                case 'i': case 'I':
                    seg.Decl = "    " + std::string(ct) + " " + var +
                               " = (" + ct + ")luaL_checkinteger(L, " +
                               std::to_string(luaIdx) + ");\n";
                    break;
                case 'f': case 'F':
                    seg.Decl = "    " + std::string(ct) + " " + var +
                               " = (" + ct + ")luaL_checknumber(L, " +
                               std::to_string(luaIdx) + ");\n";
                    break;
                }
                seg.CallArgs.push_back(var);
            }
            ++luaIdx;
            ++callIdx;
            segments.push_back(std::move(seg));
        }

        // Emit sret buffer for struct returns (int32_t ensures 4-byte alignment).
        if (retIsStruct)
            fprintf(out, "    int32_t _sret[8];\n");

        // Emit declarations.
        for (const auto& seg : segments)
            if (!seg.Decl.empty()) fprintf(out, "%s", seg.Decl.c_str());

        // Emit the import call.
        char retChar = retIsStruct ? 'v' : WasmTypeChar(mi.GetDesc().Return.Type);
        bool hasScalarReturn = (!retIsStruct && retChar != 'v' && CType(retChar) != nullptr);
        bool isBoolReturn = (mi.GetDesc().Return.Type.Primitive == EGenericValueType::Bool);

        if (hasScalarReturn)
            fprintf(out, "    %s _r = %s(",
                    CType(retChar), ImportFuncName(mi.Namespace, mi.Name).c_str());
        else
            fprintf(out, "    %s(", ImportFuncName(mi.Namespace, mi.Name).c_str());

        bool firstArg = true;
        auto sep = [&]() { if (!firstArg) fprintf(out, ", "); firstArg = false; };

        if (retIsStruct) { sep(); fprintf(out, "(int32_t)(uintptr_t)_sret"); }

        for (const auto& seg : segments)
            for (const auto& arg : seg.CallArgs) { sep(); fprintf(out, "%s", arg.c_str()); }

        fprintf(out, ");\n");

        // Emit return push.
        if (retIsStruct)
        {
            // Pack struct from sret buffer.
            const char* cn = mi.GetDesc().Return.Type.Descriptor->GetCName();
            const StructInfo* si = structByCName.count(cn) ? structByCName[cn] : nullptr;
            if (si)
            {
                fprintf(out, "    _phx_pack_%s(L", cn);
                size_t slotIdx = 0;
                for (const auto& f : si->Fields)
                {
                    if (f.WasmChar == 'I')
                        fprintf(out, ", *(int64_t*)&_sret[%zu]", slotIdx);
                    else if (f.WasmChar == 'F')
                        fprintf(out, ", *(double*)&_sret[%zu]", slotIdx);
                    else if (f.WasmChar == 'f')
                        fprintf(out, ", *(float*)&_sret[%zu]", slotIdx);
                    else
                        fprintf(out, ", _sret[%zu]", slotIdx);  // int32_t
                    slotIdx += (f.WasmChar == 'I' || f.WasmChar == 'F') ? 2 : 1;
                }
                fprintf(out, ");\n    return 1;\n");
            }
            else { fprintf(out, "    return 0;\n"); }
        }
        else if (hasScalarReturn)
        {
            const bool retIsFixed = (mi.GetDesc().Return.Type.Primitive == EGenericValueType::FixedPoint);
            int retFracBits = 0;
            if (retIsFixed && mi.GetDesc().Return.Type.Descriptor)
            {
                auto it = mi.GetDesc().Return.Type.Descriptor->GetMetadata().find("FractionalBits");
                if (it != mi.GetDesc().Return.Type.Descriptor->GetMetadata().end())
                    retFracBits = std::stoi(it->second);
            }

            if (isBoolReturn)
                fprintf(out, "    lua_pushboolean(L, (int)_r);\n");
            else if (retIsFixed && retFracBits > 0)
                fprintf(out, "    lua_pushnumber(L, (lua_Number)_r / %d.0);\n",
                        1 << retFracBits);
            else if (retChar == 'i' || retChar == 'I')
                fprintf(out, "    lua_pushinteger(L, (lua_Integer)_r);\n");
            else
                fprintf(out, "    lua_pushnumber(L, (lua_Number)_r);\n");
            fprintf(out, "    return 1;\n");
        }
        else
        {
            fprintf(out, "    return 0;\n");
        }
        fprintf(out, "}\n\n");
    }

    // Build namespace tree for table registration.
    std::map<std::string, NSNode> roots;
    for (const auto& mi : methods)
    {
        std::vector<std::string> parts = SplitDot(mi.Namespace);
        NSNode& rootNode = roots[parts[0]];
        InsertIntoTree(rootNode, parts, mi.Name, LuaFuncName(mi.Namespace, mi.Name));
    }

    // Emit register_phoenix_api().
    // Not static: lua_wasm.c declares it as extern and calls it as a cross-TU function.
    fprintf(out,
        "/* register_phoenix_api — builds nested namespace tables in the Lua state. */\n"
        "void register_phoenix_api(lua_State* L)\n"
        "{\n"
    );

    for (const auto& [rootName, rootNode] : roots)
    {
        fprintf(out, "    lua_newtable(L);\n");
        EmitTableRegistration(out, rootNode, 1);
        fprintf(out, "    lua_setglobal(L, \"%s\");\n", rootName.c_str());
    }

    EmitStructTypeTableRegistration(out, structs);

    fprintf(out, "}\n");
}

// ── main ──────────────────────────────────────────────────────────────────────

static void PrintUsage(const char* argv0)
{
    fprintf(stderr,
        "Usage: %s --host-api <path> --lua-bridge <path>\n"
        "  --host-api   Path to write host_api.h (language-agnostic WASM imports)\n"
        "  --lua-bridge Path to write lua_bridge.c (Lua stack marshaling)\n",
        argv0);
}

int main(int argc, char* argv[])
{
    const char* hostApiPath   = nullptr;
    const char* luaBridgePath = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--host-api") == 0 && i + 1 < argc)
            hostApiPath = argv[++i];
        else if (strcmp(argv[i], "--lua-bridge") == 0 && i + 1 < argc)
            luaBridgePath = argv[++i];
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            PrintUsage(argv[0]);
            return 1;
        }
    }

    if (!hostApiPath && !luaBridgePath)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    const TypeDescriptor* worldDesc = &World::GetStaticTypeDescriptor();
    std::vector<MethodInfo> methods = CollectMethods(worldDesc);
    std::vector<StructInfo> structs = CollectStructTypes(worldDesc);

    if (hostApiPath)
    {
        FILE* f = fopen(hostApiPath, "w");
        if (!f) { fprintf(stderr, "Cannot open '%s'\n", hostApiPath); return 1; }
        GenerateHostApi(f, methods, worldDesc);
        fclose(f);
    }

    if (luaBridgePath)
    {
        FILE* f = fopen(luaBridgePath, "w");
        if (!f) { fprintf(stderr, "Cannot open '%s'\n", luaBridgePath); return 1; }
        GenerateLuaBridge(f, methods, structs, worldDesc);
        fclose(f);
    }

    return 0;
}
