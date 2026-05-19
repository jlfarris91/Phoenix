/*
 * PhoenixLuaGen.cpp — Lua bridge code generator.
 *
 * Usage:
 *   PhoenixLuaGen --lua-bridge <out/lua_bridge.c>
 *
 * Output:
 *   lua_bridge.c — Lua-specific marshaling: one static l_* function per method
 *                  that unpacks Lua stack args, calls the corresponding import,
 *                  and pushes the result.  Also emits register_phoenix_api()
 *                  which builds the nested namespace tables in the Lua state.
 *
 * How registrations are discovered:
 *   CMakeLists.txt links with /WHOLEARCHIVE for every library, which forces the
 *   linker to include ALL object files regardless of symbol references.  Every
 *   PHX_DEFINE_TYPE static initializer therefore runs at startup and populates
 *   the TypeRegistry automatically.
 *
 * Adding a new module:
 *   Add a Namespace() + StaticMethod() registration to the type's PHX_DEFINE_TYPE
 *   block.  No changes to this file are needed.
 */

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "Phoenix.Sim.Script/WasmUtility.h"
#include "Phoenix/Reflection/Variant.h"
#include "Phoenix/Reflection/TypeRegistry.h"
#include "Phoenix.Sim/Scripting/IScriptBindings.h"
#include "Phoenix.Sim/Scripting/ScriptModuleBuilder.h"
#include "Phoenix.Sim/Worlds.h"

using namespace Phoenix;
using json = nlohmann::json;

// ── WASM type helpers ─────────────────────────────────────────────────────────

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

static std::string LuaFuncName(const std::string& ns, const std::string& method)
{
    return "_phx_l_" + ImportFuncName(ns, method).substr(4);
}

// "Phoenix.Unit" → "Phoenix_Unit"  (C-safe identifier component)
static std::string SanitizeNs(const std::string& s)
{
    std::string r;
    for (char c : s)
        r += (c == '.') ? '_' : c;
    return r;
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

// ── Struct helpers ────────────────────────────────────────────────────────────

struct StructField
{
    std::string Name;
    char        WasmChar;
    size_t      ByteOffset;
    int         FractionalBits;
    bool        IsFName = false;
};

static std::vector<StructField> FlattenStructFields(const TypeDescriptor& desc,
                                                     const std::string& prefix = "")
{
    std::vector<StructField> result;
    size_t offset = 0;
    for (const auto& [name, prop] : desc.GetFields())
    {
        const std::string qualName = prefix.empty() ? name : prefix + "_" + name;
        const TypeDescriptor* tr = prop.GetType();
        assert(tr);

        if (Script::IsExpandableStruct(*tr) && tr != &desc)
        {
            auto nested = FlattenStructFields(*tr, qualName);
            for (auto& f : nested) { f.ByteOffset += offset; result.push_back(f); }
            offset += tr->GetSize();
        }
        else
        {
            char c = Script::ToWasmTypeChar(*tr);
            if (c == 'v') continue;
            const size_t sz = (c == 'I' || c == 'F') ? 8 : 4;
            int fracBits = 0;
            if (tr->IsTemplate("Phoenix::TFixed"))
            {
                auto it = prop.GetMetadata().find("FractionalBits");
                if (it != prop.GetMetadata().end())
                    fracBits = std::stoi(it->second);
                else
                {
                    auto it2 = tr->GetMetadata().find("FractionalBits");
                    if (it2 != tr->GetMetadata().end())
                        fracBits = std::stoi(it2->second);
                }
            }
            bool isFName = (tr->GetTypeId() == StaticTypeName<FName>::TypeId);
            result.push_back({ qualName, c, offset, fracBits, isFName });
            offset += sz;
        }
    }
    return result;
}


struct StructInfo
{
    std::string              CName;
    std::string              LuaPath;
    std::vector<StructField> Fields;
    const TypeDescriptor*    Desc;
};

static std::vector<StructInfo> CollectStructTypes(const TypeDescriptor* worldDesc)
{
    std::vector<StructInfo> structs;
    for (const auto& [_, desc] : TypeRegistry::GetAll())
    {
        if (!desc || desc.get() == worldDesc) continue;
        if (desc->IsScriptHidden()) continue;
        if (desc->GetFields().empty()) continue;
        if (desc->GetSize() > 32) continue;
        // ScriptOptional<T> is handled via a special nil-or-value path in BuildMethodBody;
        // it must not generate pack/unpack/new/tostring helpers.
        if (desc->IsTemplate("Phoenix::ScriptOptional")) continue;

        auto fields = FlattenStructFields(*desc);
        if (fields.empty()) continue;

        StructInfo si;
        si.CName   = desc->GetAliasOrName();
        si.LuaPath = desc->GetScriptNamespace();
        si.Fields  = std::move(fields);
        si.Desc    = desc.get();
        structs.push_back(std::move(si));
    }
    std::sort(structs.begin(), structs.end(),
        [](const StructInfo& a, const StructInfo& b) { return a.CName < b.CName; });
    return structs;
}

static std::string NamespaceOfDesc(const TypeDescriptor& desc)
{
    return desc.GetScriptNamespace();
}

// ── MethodInfo ────────────────────────────────────────────────────────────────

struct MethodInfo
{
    std::string              Namespace;
    std::string              Name;
    const MethodDescriptor*  Desc      = nullptr;
    std::optional<MethodDescriptor> OwnedDesc;
    bool                     IsInstanceMethod = false; // first non-World param is a class handle
    std::string              HandleNs;                 // sanitized namespace of the owning class

    const MethodDescriptor& GetDesc() const { return OwnedDesc ? *OwnedDesc : *Desc; }

    MethodInfo() = default;
    MethodInfo(const MethodInfo& o)
        : Namespace(o.Namespace), Name(o.Name), OwnedDesc(o.OwnedDesc)
        , IsInstanceMethod(o.IsInstanceMethod), HandleNs(o.HandleNs)
    { Desc = OwnedDesc ? &*OwnedDesc : o.Desc; }
    MethodInfo(MethodInfo&& o) noexcept
        : Namespace(std::move(o.Namespace)), Name(std::move(o.Name))
        , Desc(o.Desc), OwnedDesc(std::move(o.OwnedDesc))
        , IsInstanceMethod(o.IsInstanceMethod), HandleNs(std::move(o.HandleNs))
    { if (OwnedDesc) Desc = &*OwnedDesc; }
    MethodInfo& operator=(const MethodInfo& o)
    {
        Namespace = o.Namespace; Name = o.Name; OwnedDesc = o.OwnedDesc;
        Desc = OwnedDesc ? &*OwnedDesc : o.Desc;
        IsInstanceMethod = o.IsInstanceMethod; HandleNs = o.HandleNs;
        return *this;
    }
    MethodInfo& operator=(MethodInfo&& o) noexcept
    {
        Namespace = std::move(o.Namespace); Name = std::move(o.Name);
        OwnedDesc = std::move(o.OwnedDesc); Desc = o.Desc;
        IsInstanceMethod = o.IsInstanceMethod; HandleNs = std::move(o.HandleNs);
        if (OwnedDesc) Desc = &*OwnedDesc;
        return *this;
    }
};

// ── ClassInfo ─────────────────────────────────────────────────────────────────
//
// Carries the data for a ScriptClassEntry after bindings collection.
// LuaGen emits pack/unpack helpers and a metatable for each ClassInfo.

struct ClassInfo
{
    std::string              Namespace;      // "Phoenix.Unit"
    std::string              BaseNamespace;  // "Phoenix.Entity" (empty = no base)
    std::string              HandleNs;       // "Phoenix_Unit" (sanitized for C names)
    FName                    HandleTypeId;   // TypeId of the wrapped handle (e.g. UnitId)
    std::vector<MethodInfo>  Methods;        // instance methods
    std::vector<MethodInfo>  Statics;        // static methods
};

static std::vector<MethodInfo> CollectMethods(const TypeDescriptor* worldDesc)
{
    std::vector<const TypeDescriptor*> types;
    for (const auto& [_, desc] : TypeRegistry::GetAll())
    {
        if (!desc || desc->IsScriptHidden()) continue;

        bool hasStaticMethods = false;
        for (const auto& [n, m] : desc->GetMethods())
            if (m.IsStatic() && !m.IsScriptHidden()) { hasStaticMethods = true; break; }

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
        for (const auto& [mname, m] : desc->GetMethods())
        {
            if (!m.IsStatic() || m.IsScriptHidden()) continue;
            MethodInfo mi;
            mi.Namespace = ns;
            mi.Name      = mname;
            mi.Desc      = &m;
            methods.push_back(std::move(mi));
        }
    }
    return methods;
}

// Discover all IScriptBindings implementations, instantiate each, call Describe(),
// then append their entries to `methods` and `classes`.
static void CollectBindings(std::vector<MethodInfo>& methods, std::vector<ClassInfo>& classes)
{
    ScriptModuleBuilder builder;
    for (const TypeDescriptor* bindDesc : TypeRegistry::GetAllDerivedFrom<IScriptBindings>())
    {
        if (!bindDesc || bindDesc->GetSize() == 0) continue;
        std::vector<uint8_t> storage(bindDesc->GetSize());
        bindDesc->DefaultConstruct(storage.data());
        reinterpret_cast<IScriptBindings*>(storage.data())->Describe(builder);
        bindDesc->Destruct(storage.data());
    }

    // Free functions → MethodInfo (same as reflection-registered methods)
    for (const auto& entry : builder.GetFunctions())
    {
        MethodInfo mi;
        mi.Namespace = entry.Namespace;
        mi.OwnedDesc = entry.Method;
        mi.Desc      = &*mi.OwnedDesc;
        mi.Name      = mi.GetDesc().GetName();
        methods.push_back(std::move(mi));
    }

    // Class entries → ClassInfo + flattened MethodInfo for each method/static
    for (const auto& cls : builder.GetClasses())
    {
        ClassInfo* ci = nullptr;
        for (ClassInfo& existingCi : classes)
        {
            if (existingCi.Namespace == cls.Namespace)
            {
                assert(existingCi.HandleTypeId == cls.HandleTypeId);
                ci = &existingCi;
                break;
            }
        }

        if (!ci)
        {
            ci = &classes.emplace_back();
        }

        ci->Namespace     = cls.Namespace;
        ci->BaseNamespace = cls.BaseNamespace;
        ci->HandleNs      = SanitizeNs(cls.Namespace);
        ci->HandleTypeId  = cls.HandleTypeId;

        auto makeMethod = [&](const MethodDescriptor& m, bool isInstance) -> MethodInfo
        {
            MethodInfo mi;
            mi.Namespace        = cls.Namespace;
            mi.OwnedDesc        = m;
            mi.Desc             = &*mi.OwnedDesc;
            mi.Name             = mi.GetDesc().GetName();
            mi.IsInstanceMethod = isInstance;
            mi.HandleNs         = ci->HandleNs;
            return mi;
        };

        // Insert into flat methods list, replacing any existing reflection entry with
        // the same (namespace, name) so the IScriptBindings version (with IsInstanceMethod)
        // is used and no duplicate declarations appear in the generated C files.
        auto upsert = [&](MethodInfo mi)
        {
            auto it = std::find_if(methods.begin(), methods.end(),
                [&](const MethodInfo& e) { return e.Namespace == mi.Namespace && e.Name == mi.Name; });
            if (it != methods.end()) *it = mi;
            else methods.push_back(mi);
        };

        for (const auto& m : cls.Methods)
        {
            ci->Methods.push_back(makeMethod(m, true));
            upsert(ci->Methods.back());
        }
        for (const auto& s : cls.Statics)
        {
            ci->Statics.push_back(makeMethod(s, false));
            upsert(ci->Statics.back());
        }
    }
}

// ── Namespace tree (for register_phoenix_api body) ────────────────────────────

struct NSNode
{
    std::vector<std::pair<std::string, std::string>> Methods; // {lua_name, c_func_name}
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

static void EmitTableRegistration(std::string& out, const NSNode& node, int indent)
{
    std::string pad(indent * 4, ' ');
    for (const auto& [luaName, cFunc] : node.Methods)
    {
        out += pad + "lua_pushcfunction(L, " + cFunc + ");\n";
        out += pad + "lua_setfield(L, -2, \"" + luaName + "\");\n";
    }
    for (const auto& [childName, child] : node.Children)
    {
        out += pad + "lua_getfield(L, -1, \"" + childName + "\");\n";
        out += pad + "if (!lua_istable(L, -1)) { lua_pop(L, 1); lua_newtable(L); }\n";
        EmitTableRegistration(out, child, indent);
        out += pad + "lua_setfield(L, -2, \"" + childName + "\");\n";
    }
}

static void EmitPlaceAtLuaPath(std::string& out, const std::string& luaPath, int indent)
{
    std::string pad(indent * 4, ' ');
    auto parts = SplitDot(luaPath);

    if (parts.size() == 1)
    {
        out += pad + "lua_setglobal(L, \"" + parts[0] + "\");\n";
        return;
    }

    out += pad + "lua_getglobal(L, \"" + parts[0] + "\");\n";
    out += pad + "if (lua_isnil(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
               + "lua_pushvalue(L, -1); lua_setglobal(L, \"" + parts[0] + "\"); }\n";

    for (size_t i = 1; i + 1 < parts.size(); ++i)
    {
        out += pad + "lua_getfield(L, -1, \"" + parts[i] + "\");\n";
        out += pad + "if (lua_isnil(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
                   + "lua_pushvalue(L, -1); lua_setfield(L, -3, \"" + parts[i] + "\"); }\n";
        out += pad + "lua_remove(L, -2);\n";
    }

    out += pad + "lua_insert(L, -2);\n";
    out += pad + "lua_setfield(L, -2, \"" + parts.back() + "\");\n";
    out += pad + "lua_pop(L, 1);\n";
}

// Navigate to (or create) the table at luaPath, leaving it on the stack top.
// Caller must pop it when done.
static void EmitGetOrCreateTable(std::string& out, const std::string& luaPath, int indent)
{
    std::string pad(indent * 4, ' ');
    auto parts = SplitDot(luaPath);

    out += pad + "lua_getglobal(L, \"" + parts[0] + "\");\n";
    out += pad + "if (!lua_istable(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
               + "lua_pushvalue(L, -1); lua_setglobal(L, \"" + parts[0] + "\"); }\n";

    for (size_t i = 1; i < parts.size(); ++i)
    {
        out += pad + "lua_getfield(L, -1, \"" + parts[i] + "\");\n";
        out += pad + "if (!lua_istable(L, -1)) { lua_pop(L, 1); lua_newtable(L); "
                   + "lua_pushvalue(L, -1); lua_setfield(L, -3, \"" + parts[i] + "\"); }\n";
        out += pad + "lua_remove(L, -2);\n";
    }
}

static std::string BuildRegisterBody(const std::vector<MethodInfo>& methods,
                                      const std::vector<StructInfo>& structs,
                                      const std::vector<ClassInfo>& classes)
{
    // Build namespace tree for method tables.
    std::map<std::string, NSNode> roots;
    for (const auto& mi : methods)
    {
        std::vector<std::string> parts = SplitDot(mi.Namespace);
        NSNode& rootNode = roots[parts[0]];
        InsertIntoTree(rootNode, parts, mi.Name, LuaFuncName(mi.Namespace, mi.Name));
    }

    std::string out;

    for (const auto& [rootName, rootNode] : roots)
    {
        out += "    lua_getglobal(L, \"" + rootName + "\");\n";
        out += "    if (!lua_istable(L, -1)) { lua_pop(L, 1); lua_newtable(L); }\n";
        EmitTableRegistration(out, rootNode, 1);
        out += "    lua_setglobal(L, \"" + rootName + "\");\n";
    }

    if (!structs.empty())
    {
        out += "\n    /* ── Struct type tables ── */\n";
        for (const auto& si : structs)
        {
            out += "\n    /* " + si.CName + " */\n";
            // Get or create the table at this path (may already exist with static methods).
            EmitGetOrCreateTable(out, si.LuaPath, 1);
            out += "    lua_pushvalue(L, -1); lua_setfield(L, -2, \"__index\");\n";
            out += "    lua_pushstring(L, \"" + si.CName + "\"); lua_setfield(L, -2, \"__type\");\n";
            out += "    lua_pushcfunction(L, _phx_" + si.CName + "_tostring);"
                   " lua_setfield(L, -2, \"__tostring\");\n";
            out += "    lua_pushcfunction(L, _phx_" + si.CName + "_new);"
                   " lua_setfield(L, -2, \"new\");\n";
            out += "    lua_pushvalue(L, -1);\n";
            out += "    lua_setfield(L, LUA_REGISTRYINDEX, \"_phx_mt_" + si.CName + "\");\n";
            out += "    lua_pop(L, 1);\n";
        }
    }

    if (!classes.empty())
    {
        out += "\n    /* ── Class metatables ── */\n";
        for (const auto& ci : classes)
        {
            out += "\n    /* " + ci.Namespace + " (OOP class) */\n";
            // Get or create the table at the class namespace path.
            EmitGetOrCreateTable(out, ci.Namespace, 1);
            // Register instance methods in the table.
            for (const auto& m : ci.Methods)
            {
                out += "    lua_pushcfunction(L, " + LuaFuncName(ci.Namespace, m.Name) + ");\n";
                out += "    lua_setfield(L, -2, \"" + m.Name + "\");\n";
            }
            // Register static methods in the table.
            for (const auto& s : ci.Statics)
            {
                out += "    lua_pushcfunction(L, " + LuaFuncName(ci.Namespace, s.Name) + ");\n";
                out += "    lua_setfield(L, -2, \"" + s.Name + "\");\n";
            }
            // __index = self, enabling unit:Method() syntax.
            out += "    lua_pushvalue(L, -1); lua_setfield(L, -2, \"__index\");\n";
            // Store metatable in registry so pack functions can attach it.
            out += "    lua_pushvalue(L, -1);\n";
            out += "    lua_setfield(L, LUA_REGISTRYINDEX, \"_phx_mt_" + ci.HandleNs + "\");\n";
            out += "    lua_pop(L, 1);\n";
        }
    }

    // ── Class inheritance ─────────────────────────────────────────────────────
    // Done in a second pass so both base and derived metatables are already set up.
    // Sets setmetatable(Derived, {__index = Base}) so instance method lookup falls
    // through from Derived to Base when a method isn't found locally.
    for (const auto& ci : classes)
    {
        if (ci.BaseNamespace.empty()) continue;
        out += "\n    /* " + ci.Namespace + " : " + ci.BaseNamespace + " */\n";
        EmitGetOrCreateTable(out, ci.Namespace, 1);      // [Derived]
        out += "    lua_newtable(L);\n";                 // [Derived, {}]
        EmitGetOrCreateTable(out, ci.BaseNamespace, 1);  // [Derived, {}, Base]
        out += "    lua_setfield(L, -2, \"__index\");\n"; // [Derived, {__index=Base}]
        out += "    lua_setmetatable(L, -2);\n";          // [Derived]
        out += "    lua_pop(L, 1);\n";                    // []
    }

    out += "\n    /* ── Phoenix utilities ── */\n";
    out += "    lua_getglobal(L, \"Phoenix\");\n";
    out += "    if (lua_istable(L, -1)) {\n";
    out += "        lua_pushcfunction(L, _phx_l_hash); lua_setfield(L, -2, \"Hash\");\n";
    out += "    }\n";
    out += "    lua_pop(L, 1);\n";

    return out;
}

// ── Method body builder ───────────────────────────────────────────────────────
//
// Pre-computes the full function body (declarations + call + return push) for
// each method wrapper.  The inja template simply drops m.body into the function.

static std::string BuildMethodBody(const MethodInfo& mi,
                                    const TypeDescriptor* worldDesc,
                                    const std::map<std::string, const StructInfo*>& structByCName,
                                    const std::map<FName, std::string>& handleNsByTypeId)
{
    struct Segment {
        std::string              Decl;
        std::vector<std::string> CallArgs;
    };

    const TypeDescriptor& returnType = *mi.GetDesc().GetReturnType();
    const bool retIsStruct = Script::IsExpandableStruct(returnType);

    std::vector<Segment> segments;
    int luaIdx  = 1;
    int callIdx = 0;
    bool expectingHandle = mi.IsInstanceMethod; // consume handle from self at first non-World param

    for (const auto& p : mi.GetDesc().GetParams())
    {
        if (p.Type == worldDesc) continue;

        // ── Instance method handle param ───────────────────────────────────────
        // The first non-World param of an instance method is the class handle.
        // It lives in a Lua table as the "_id" field (set by _phx_pack_<HandleNs>_handle).
        if (expectingHandle)
        {
            expectingHandle = false;
            std::string var = "p" + std::to_string(callIdx);
            Segment seg;
            seg.Decl = "    int32_t " + var + " = _phx_unpack_" + mi.HandleNs +
                       "_handle(L, " + std::to_string(luaIdx) + ");\n";
            seg.CallArgs.push_back(var);
            ++luaIdx; ++callIdx;
            segments.push_back(std::move(seg));
            continue;
        }

        if (Script::IsExpandableStruct(*p.Type))
        {
            const char* cn = p.Type->GetAliasOrName().c_str();
            auto it = structByCName.find(cn);
            if (it == structByCName.end()) { ++luaIdx; continue; }
            const StructInfo* si = it->second;

            Segment seg;
            for (const auto& f : si->Fields)
                seg.Decl += std::string("    ") + CType(f.WasmChar) +
                            " p" + std::to_string(callIdx) + "_" + f.Name + ";\n";
            seg.Decl += "    _phx_unpack_" + std::string(cn) +
                        "(L, " + std::to_string(luaIdx);
            for (const auto& f : si->Fields)
                seg.Decl += ", &p" + std::to_string(callIdx) + "_" + f.Name;
            seg.Decl += ");\n";
            for (const auto& f : si->Fields)
                seg.CallArgs.push_back("p" + std::to_string(callIdx) + "_" + f.Name);
            callIdx += (int)si->Fields.size();
            ++luaIdx;
            segments.push_back(std::move(seg));
            continue;
        }

        char c = Script::ToWasmTypeChar(*p.Type);
        if (c == 'v') { ++luaIdx; continue; }

        bool isFName = p.Type->GetTypeId() == StaticTypeName<FName>::TypeId;
        bool isFixed = p.Type->IsTemplate("Phoenix::TFixed");
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
        else if (isFixed && p.Type)
        {
            int fracBits = 0;
            auto it = p.Type->GetMetadata().find("FractionalBits");
            if (it != p.Type->GetMetadata().end())
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

    std::string body;

    if (retIsStruct)
        body += "    int32_t _sret[8];\n";

    for (const auto& seg : segments)
        if (!seg.Decl.empty()) body += seg.Decl;

    char retChar = retIsStruct ? 'v' : Script::ToWasmTypeChar(returnType);
    bool hasScalarReturn = !retIsStruct && retChar != 'v' && CType(retChar) != nullptr;
    bool isBoolReturn    = returnType.GetTypeId() == StaticTypeName<bool>::TypeId;

    // Call expression.
    if (hasScalarReturn)
        body += "    " + std::string(CType(retChar)) + " _r = " +
                ImportFuncName(mi.Namespace, mi.Name) + "(";
    else
        body += "    " + ImportFuncName(mi.Namespace, mi.Name) + "(";

    bool firstArg = true;
    auto sep = [&]() { if (!firstArg) body += ", "; firstArg = false; };

    if (retIsStruct) { sep(); body += "(int32_t)(uintptr_t)_sret"; }

    for (const auto& seg : segments)
        for (const auto& arg : seg.CallArgs) { sep(); body += arg; }

    body += ");\n";

    // Return push.
    if (retIsStruct)
    {
        const char* cn = returnType.GetAliasOrName().c_str();
        auto it = structByCName.find(cn);

        // ── ScriptOptional<T>: emit nil-or-value instead of a Lua table ──────
        if (it != structByCName.end() && returnType.IsTemplate("Phoenix::ScriptOptional"))
        {
            const StructInfo* si = it->second;

            // Find slot indices for HasValue and Value by field name.
            int hasValueSlot = 0, valueSlot = 1;
            StructField valueField{};
            {
                size_t slotIdx = 0;
                for (const auto& f : si->Fields)
                {
                    if (f.Name == "HasValue") hasValueSlot = (int)slotIdx;
                    if (f.Name == "Value")    { valueSlot = (int)slotIdx; valueField = f; }
                    slotIdx += (f.WasmChar == 'I' || f.WasmChar == 'F') ? 2 : 1;
                }
            }

            body += "    if (_sret[" + std::to_string(hasValueSlot) + "]) {\n";

            // Push value — check whether it's a class handle first.
            const TypeDescriptor* valueType = nullptr;
            {
                auto vf = returnType.GetFields().find("Value");
                if (vf != returnType.GetFields().end()) valueType = vf->second.GetType();
            }
            bool pushedHandle = false;
            if (valueType)
            {
                auto handleIt = handleNsByTypeId.find(valueType->GetTypeId());
                if (handleIt != handleNsByTypeId.end())
                {
                    body += "        _phx_pack_" + handleIt->second +
                            "_handle(L, (int32_t)_sret[" + std::to_string(valueSlot) + "]);\n";
                    pushedHandle = true;
                }
            }
            if (!pushedHandle)
            {
                if (valueField.WasmChar == 'I')
                    body += "        lua_pushinteger(L, (lua_Integer)*(int64_t*)&_sret[" + std::to_string(valueSlot) + "]);\n";
                else if (valueField.WasmChar == 'f')
                    body += "        lua_pushnumber(L, (lua_Number)*(float*)&_sret[" + std::to_string(valueSlot) + "]);\n";
                else if (valueField.WasmChar == 'F')
                    body += "        lua_pushnumber(L, (lua_Number)*(double*)&_sret[" + std::to_string(valueSlot) + "]);\n";
                else
                    body += "        lua_pushinteger(L, (lua_Integer)_sret[" + std::to_string(valueSlot) + "]);\n";
            }

            body += "    } else {\n        lua_pushnil(L);\n    }\n    return 1;\n";
        }
        // ── Regular struct: pack into a Lua table ────────────────────────────
        else if (it != structByCName.end())
        {
            const StructInfo* si = it->second;
            body += "    _phx_pack_" + std::string(cn) + "(L";
            size_t slotIdx = 0;
            for (const auto& f : si->Fields)
            {
                if (f.WasmChar == 'I')
                    body += ", *(int64_t*)&_sret[" + std::to_string(slotIdx) + "]";
                else if (f.WasmChar == 'F')
                    body += ", *(double*)&_sret[" + std::to_string(slotIdx) + "]";
                else if (f.WasmChar == 'f')
                    body += ", *(float*)&_sret[" + std::to_string(slotIdx) + "]";
                else
                    body += ", _sret[" + std::to_string(slotIdx) + "]";
                slotIdx += (f.WasmChar == 'I' || f.WasmChar == 'F') ? 2 : 1;
            }
            body += ");\n    return 1;\n";
        }
        else { body += "    return 0;\n"; }
    }
    else if (hasScalarReturn)
    {
        // Check whether the return type is a class handle — if so, pack it as a table.
        auto handleIt = handleNsByTypeId.find(returnType.GetTypeId());
        if (handleIt != handleNsByTypeId.end())
        {
            body += "    _phx_pack_" + handleIt->second + "_handle(L, (int32_t)_r);\n";
            body += "    return 1;\n";
        }
        else
        {
            const bool retIsFixed = returnType.IsTemplate("Phoenix::TFixed");
            int retFracBits = 0;
            if (retIsFixed)
            {
                auto it = returnType.GetMetadata().find("FractionalBits");
                if (it != returnType.GetMetadata().end())
                    retFracBits = std::stoi(it->second);
            }

            if (isBoolReturn)
                body += "    lua_pushboolean(L, (int)_r);\n";
            else if (retIsFixed && retFracBits > 0)
                body += "    lua_pushnumber(L, (lua_Number)_r / " +
                        std::to_string(1 << retFracBits) + ".0);\n";
            else if (retChar == 'i' || retChar == 'I')
                body += "    lua_pushinteger(L, (lua_Integer)_r);\n";
            else
                body += "    lua_pushnumber(L, (lua_Number)_r);\n";
            body += "    return 1;\n";
        }
    }
    else
    {
        body += "    return 0;\n";
    }

    return body;
}

// ── inja templates ────────────────────────────────────────────────────────────

static constexpr const char* kLuaBridgeTemplate = R"TMPL(/* Auto-generated by PhoenixLuaGen — do not edit.
 * Regenerate: build the PhoenixLuaGen target.
 *
 * Provides:
 *   • One static l_* Lua C function per registered host method.
 *   • register_phoenix_api(lua_State*) — builds the nested Phoenix.*
 *     namespace tables in the Lua state.
 */

#include "host_api.h"
#include "lua.h"
#include "lauxlib.h"
#include <stdint.h>
#include <string.h>

static uint32_t _phx_fnv1a32(const char* s, size_t n)
{
    uint32_t h = 0x811c9dc5u;
    while (n--) { h ^= (uint8_t)*s++; h *= 0x01000193u; }
    return h;
}

static int _phx_l_hash(lua_State* L)
{
    const char* s = luaL_checkstring(L, 1);
    lua_pushinteger(L, (lua_Integer)_phx_fnv1a32(s, strlen(s)));
    return 1;
}
{% if length(structs) > 0 %}

/* ── Struct pack/unpack helpers ──────────────────────────────────── */
{% for s in structs %}

static void _phx_unpack_{{ s.cname }}(lua_State* L, int idx{% for f in s.fields %}, {{ f.ctype }}* {{ f.name }}{% endfor %})
{
{% for f in s.fields %}
{% if f.is_fixed %}
    lua_getfield(L, idx, "{{ f.name }}"); *{{ f.name }} = ({{ f.ctype }})(lua_tonumber(L, -1) * {{ f.scale }}.0); lua_pop(L, 1);
{% else if f.is_float %}
    lua_getfield(L, idx, "{{ f.name }}"); *{{ f.name }} = ({{ f.ctype }})lua_tonumber(L, -1); lua_pop(L, 1);
{% else if f.is_fname %}
    lua_getfield(L, idx, "{{ f.name }}"); if (lua_type(L, -1) == LUA_TSTRING) { const char* _s = lua_tostring(L, -1); *{{ f.name }} = ({{ f.ctype }})_phx_fnv1a32(_s, strlen(_s)); } else { *{{ f.name }} = ({{ f.ctype }})lua_tointeger(L, -1); } lua_pop(L, 1);
{% else %}
    lua_getfield(L, idx, "{{ f.name }}"); *{{ f.name }} = ({{ f.ctype }})lua_tointeger(L, -1); lua_pop(L, 1);
{% endif %}
{% endfor %}
}

static void _phx_pack_{{ s.cname }}(lua_State* L{% for f in s.fields %}, {{ f.ctype }} {{ f.name }}{% endfor %})
{
    lua_newtable(L);
{% for f in s.fields %}
{% if f.is_fixed %}
    lua_pushnumber(L, (lua_Number){{ f.name }} / {{ f.scale }}.0); lua_setfield(L, -2, "{{ f.name }}");
{% else if f.is_float %}
    lua_pushnumber(L, (lua_Number){{ f.name }}); lua_setfield(L, -2, "{{ f.name }}");
{% else %}
    lua_pushinteger(L, (lua_Integer){{ f.name }}); lua_setfield(L, -2, "{{ f.name }}");
{% endif %}
{% endfor %}
    lua_getfield(L, LUA_REGISTRYINDEX, "_phx_mt_{{ s.cname }}");
    if (lua_istable(L, -1)) lua_setmetatable(L, -2);
    else lua_pop(L, 1);
}

static int _phx_{{ s.cname }}_tostring(lua_State* L)
{
{% for f in s.fields %}
    {{ f.ctype }} {{ f.name }};
{% endfor %}
    _phx_unpack_{{ s.cname }}(L, 1{% for f in s.fields %}, &{{ f.name }}{% endfor %});
    lua_pushfstring(L, "{{ s.tostring_fmt }}"{% for f in s.fields %}, {{ f.tostring_arg }}{% endfor %});
    return 1;
}

static int _phx_{{ s.cname }}_new(lua_State* L)
{
    lua_newtable(L);
    const int tbl = lua_gettop(L);
{% for f in s.fields %}
{% if f.is_fixed or f.is_float %}
    lua_pushnumber(L, 0.0); lua_setfield(L, tbl, "{{ f.name }}");
{% else %}
    lua_pushinteger(L, 0); lua_setfield(L, tbl, "{{ f.name }}");
{% endif %}
{% endfor %}
    if (lua_istable(L, 1)) {
{% for f in s.fields %}
{% if f.is_fname %}
        lua_getfield(L, 1, "{{ f.name }}"); if (lua_type(L, -1) == LUA_TSTRING) { const char* _s = lua_tostring(L, -1); uint32_t _h = _phx_fnv1a32(_s, strlen(_s)); lua_pop(L, 1); lua_pushinteger(L, (lua_Integer)_h); } if (!lua_isnil(L, -1)) { lua_setfield(L, tbl, "{{ f.name }}"); } else { lua_pop(L, 1); }
{% else %}
        lua_getfield(L, 1, "{{ f.name }}"); if (!lua_isnil(L, -1)) { lua_setfield(L, tbl, "{{ f.name }}"); } else { lua_pop(L, 1); }
{% endif %}
{% endfor %}
    }
    lua_getfield(L, LUA_REGISTRYINDEX, "_phx_mt_{{ s.cname }}");
    if (lua_istable(L, -1)) { lua_setmetatable(L, tbl); } else { lua_pop(L, 1); }
    lua_settop(L, tbl);
    return 1;
}
{% endfor %}
{% endif %}
{% if length(classes) > 0 %}

/* ── Class handle pack/unpack helpers ── */
{% for c in classes %}

/* {{ c.namespace }} — handle type packed as {_id=N} table */
static int32_t _phx_unpack_{{ c.handle_ns }}_handle(lua_State* L, int idx)
{
    lua_getfield(L, idx, "_id");
    int32_t v = (int32_t)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

static void _phx_pack_{{ c.handle_ns }}_handle(lua_State* L, int32_t id)
{
    lua_newtable(L);
    lua_pushinteger(L, (lua_Integer)id);
    lua_setfield(L, -2, "_id");
    lua_getfield(L, LUA_REGISTRYINDEX, "_phx_mt_{{ c.handle_ns }}");
    if (lua_istable(L, -1)) lua_setmetatable(L, -2);
    else lua_pop(L, 1);
}
{% endfor %}
{% endif %}

/* ── Method wrappers ── */
{% for m in methods %}

/* {{ m.namespace }}.{{ m.name }} */
static int {{ m.lua_func }}(lua_State* L)
{
{{ m.body }}}
{% endfor %}

/* register_phoenix_api — builds nested namespace tables in the Lua state. */
void register_phoenix_api(lua_State* L)
{
{{ register_body }}}
)TMPL";

// ── data model ────────────────────────────────────────────────────────────────

static json BuildStructFieldJson(const StructField& f)
{
    bool isFloat = (f.WasmChar == 'f' || f.WasmChar == 'F');
    bool isFixed = (f.FractionalBits > 0);

    std::string tostringArg;
    if (isFixed)
        tostringArg = "(double)" + f.Name + " / " + std::to_string(1 << f.FractionalBits) + ".0";
    else if (isFloat)
        tostringArg = "(double)" + f.Name;
    else if (f.WasmChar == 'I')
        tostringArg = "(long long)" + f.Name;
    else
        tostringArg = "(int)" + f.Name;

    return {
        {"name",         f.Name},
        {"ctype",        std::string(CType(f.WasmChar))},
        {"is_float",     isFloat && !isFixed},
        {"is_fixed",     isFixed},
        {"is_fname",     f.IsFName},
        {"scale",        isFixed ? (1 << f.FractionalBits) : 0},
        {"tostring_arg", tostringArg}
    };
}

static json BuildLuaBridgeData(const std::vector<MethodInfo>& methods,
                                const std::vector<StructInfo>& structs,
                                const std::vector<ClassInfo>& classes,
                                const TypeDescriptor* worldDesc)
{
    json data;

    // ── Structs ───────────────────────────────────────────────────────────────
    data["structs"] = json::array();
    for (const auto& si : structs)
    {
        json s;
        s["cname"]    = si.CName;
        s["lua_path"] = si.LuaPath;
        s["fields"]   = json::array();

        // Build tostring format string.
        std::string fmtStr = si.CName + "{";
        for (size_t i = 0; i < si.Fields.size(); ++i)
        {
            if (i) fmtStr += ", ";
            const auto& f = si.Fields[i];
            bool isNumeric = (f.WasmChar == 'f' || f.WasmChar == 'F' || f.FractionalBits > 0);
            fmtStr += f.Name + "=" + (isNumeric ? "%g" : "%d");
        }
        fmtStr += "}";
        s["tostring_fmt"] = fmtStr;

        for (const auto& f : si.Fields)
            s["fields"].push_back(BuildStructFieldJson(f));

        data["structs"].push_back(std::move(s));
    }

    // ── Classes ───────────────────────────────────────────────────────────────
    data["classes"] = json::array();
    for (const auto& ci : classes)
        data["classes"].push_back({ {"namespace", ci.Namespace}, {"handle_ns", ci.HandleNs} });

    // Build handle type id → HandleNs map for BuildMethodBody return-type detection.
    std::map<FName, std::string> handleNsByTypeId;
    for (const auto& ci : classes)
        handleNsByTypeId[ci.HandleTypeId] = ci.HandleNs;

    // ── Methods ───────────────────────────────────────────────────────────────
    std::map<std::string, const StructInfo*> structByCName;
    for (const auto& si : structs) structByCName[si.CName] = &si;

    data["methods"] = json::array();
    for (const auto& mi : methods)
    {
        data["methods"].push_back({
            {"namespace", mi.Namespace},
            {"name",      mi.Name},
            {"lua_func",  LuaFuncName(mi.Namespace, mi.Name)},
            {"body",      BuildMethodBody(mi, worldDesc, structByCName, handleNsByTypeId)}
        });
    }

    // ── register_phoenix_api body ─────────────────────────────────────────────
    data["register_body"] = BuildRegisterBody(methods, structs, classes);

    return data;
}

// ── LuaLS definition file generator ──────────────────────────────────────────

// Maps a TypeDescriptor to a LuaLS type annotation string.
// Returns the logical Lua type — NOT the flattened WASM representation.
static std::string TypeToLuaAnnotation(const TypeDescriptor* type,
                                        const TypeDescriptor* worldDesc)
{
    if (!type || type == worldDesc) return "any";

    if (type->GetTypeId() == StaticTypeName<void>::TypeId)  return "nil";
    if (type->GetTypeId() == StaticTypeName<bool>::TypeId)  return "boolean";
    if (type->GetTypeId() == StaticTypeName<FName>::TypeId) return "integer|string";

    if (type->IsTemplate("Phoenix::TFixed")) return "number";

    switch (type->GetTypeId())
    {
        case StaticTypeName<int8>::TypeId:
        case StaticTypeName<uint8>::TypeId:
        case StaticTypeName<int16>::TypeId:
        case StaticTypeName<uint16>::TypeId:
        case StaticTypeName<int32>::TypeId:
        case StaticTypeName<uint32>::TypeId:
        case StaticTypeName<int64>::TypeId:
        case StaticTypeName<uint64>::TypeId:
            return "integer";
        case StaticTypeName<float>::TypeId:
        case StaticTypeName<double>::TypeId:
            return "number";
        default:
            break;
    }

    // ScriptOptional<T>: return "T|nil" so LuaLS knows this can be nil.
    if (type->IsTemplate("Phoenix::ScriptOptional"))
    {
        auto vf = type->GetFields().find("Value");
        if (vf != type->GetFields().end() && vf->second.GetType())
            return TypeToLuaAnnotation(vf->second.GetType(), worldDesc) + "|nil";
        return "any|nil";
    }

    // Named struct/class with registered fields → use its script namespace as the class name.
    if (!type->GetFields().empty())
        return type->GetScriptNamespace();

    // Opaque / unregistered integer-sized types (handles, enums, etc.)
    if (type->GetSize() <= sizeof(int32))
        return "integer";
    if (type->GetSize() == sizeof(int64))
        return "integer";

    return "any";
}

// Collects all script-visible types that have registered fields, regardless of size.
// Used for .d.lua — we want the full logical type list, not just ≤32-byte WASM-expandable ones.
static std::vector<const TypeDescriptor*> CollectAllScriptTypes(const TypeDescriptor* worldDesc)
{
    std::vector<const TypeDescriptor*> types;
    for (const auto& [_, desc] : TypeRegistry::GetAll())
    {
        if (!desc || desc.get() == worldDesc) continue;
        if (desc->IsScriptHidden()) continue;
        if (desc->GetFields().empty()) continue;
        types.push_back(desc.get());
    }
    std::sort(types.begin(), types.end(), [](const TypeDescriptor* a, const TypeDescriptor* b)
    { return a->GetScriptNamespace() < b->GetScriptNamespace(); });
    return types;
}

// Emits method stubs for one namespace into `out`.
static void EmitMethodStubs(std::string& out, const std::string& ns,
                             const std::vector<const MethodInfo*>& mis,
                             const TypeDescriptor* worldDesc)
{
    for (const MethodInfo* mi : mis)
    {
        out += "\n";
        // Build param names: use the registered name if it's a valid Lua identifier,
        // otherwise fall back to arg0, arg1, ... (unnamed params get numeric strings).
        auto isValidIdent = [](const std::string& s) {
            if (s.empty()) return false;
            if (!std::isalpha(static_cast<unsigned char>(s[0])) && s[0] != '_') return false;
            return std::all_of(s.begin(), s.end(), [](unsigned char c) {
                return std::isalnum(c) || c == '_';
            });
        };
        std::vector<std::string> pnames;
        int argIdx = 0;
        for (const auto& p : mi->GetDesc().GetParams())
        {
            if (p.Type == worldDesc) continue;
            pnames.push_back(isValidIdent(p.Name) ? p.Name : ("arg" + std::to_string(argIdx)));
            ++argIdx;
        }
        argIdx = 0;
        for (const auto& p : mi->GetDesc().GetParams())
        {
            if (p.Type == worldDesc) continue;
            out += "---@param " + pnames[argIdx] + " " + TypeToLuaAnnotation(p.Type, worldDesc) + "\n";
            ++argIdx;
        }
        const TypeDescriptor* ret = mi->GetDesc().GetReturnType();
        if (ret && ret->GetTypeId() != StaticTypeName<void>::TypeId)
            out += "---@return " + TypeToLuaAnnotation(ret, worldDesc) + "\n";
        out += "function " + ns + "." + mi->Name + "(";
        for (size_t i = 0; i < pnames.size(); ++i)
        {
            if (i) out += ", ";
            out += pnames[i];
        }
        out += ") end\n";
    }
}

static std::string BuildLuaDefs(const std::vector<MethodInfo>& methods,
                                  const std::vector<ClassInfo>& classes,
                                  const TypeDescriptor* worldDesc)
{
    auto allTypes = CollectAllScriptTypes(worldDesc);

    // Index methods by namespace for quick lookup.
    std::map<std::string, std::vector<const MethodInfo*>> methodsByNs;
    for (const auto& mi : methods)
        if (!mi.IsInstanceMethod)   // instance methods are emitted under the OOP class block
            methodsByNs[mi.Namespace].push_back(&mi);

    // Collect the set of namespaces that are struct types or OOP classes — these get
    // class declarations rather than bare table declarations.
    std::set<std::string> structNamespaces;
    for (const TypeDescriptor* desc : allTypes)
        structNamespaces.insert(desc->GetScriptNamespace());

    // OOP class namespaces (from IScriptBindings) also behave like struct namespaces.
    std::set<std::string> classNamespaces;
    for (const auto& ci : classes)
        classNamespaces.insert(ci.Namespace);

    // Collect every intermediate namespace prefix that needs a table declaration.
    // e.g. "Phoenix.RTS.Command" implies "Phoenix" and "Phoenix.RTS".
    std::set<std::string> allNamespaces;
    auto collectPrefixes = [&](const std::string& ns)
    {
        auto parts = SplitDot(ns);
        std::string prefix;
        for (const auto& p : parts)
        {
            if (!prefix.empty()) prefix += ".";
            prefix += p;
            allNamespaces.insert(prefix);
        }
    };
    for (const TypeDescriptor* desc : allTypes)
        collectPrefixes(desc->GetScriptNamespace());
    for (const auto& mi : methods)
        collectPrefixes(mi.Namespace);
    for (const auto& ci : classes)
        collectPrefixes(ci.Namespace);

    std::string out;
    out += "---@meta\n";
    out += "-- Auto-generated by PhoenixLuaGen — do not edit.\n";
    out += "-- Provides LuaLS type annotations for the Phoenix host API.\n";
    out += "-- Regenerate: build the PhoenixLuaGen target.\n";

    // Emit bare table declarations for pure namespace prefixes that are not themselves
    // struct classes or OOP classes (so that nested assignments like Phoenix.Vec2 = {} are valid).
    out += "\n";
    for (const auto& ns : allNamespaces)
    {
        if (structNamespaces.count(ns) == 0 && classNamespaces.count(ns) == 0)
            out += ns + " = {}\n";
    }

    // ── Struct / value-type class declarations ────────────────────────────────
    for (const TypeDescriptor* desc : allTypes)
    {
        const std::string ns = desc->GetScriptNamespace();
        out += "\n---@class " + ns + "\n";

        // Fields — use GetFields() directly so TargetLocation appears as Phoenix.Vec2,
        // not as the flattened TargetLocation_X / TargetLocation_Y used in the WASM ABI.
        for (const auto& [name, field] : desc->GetFields())
        {
            if (field.IsStatic() || field.IsScriptHidden()) continue;
            const TypeDescriptor* ft = field.GetType();
            if (!ft || ft == desc) continue;  // skip self-referential static fields
            out += "---@field " + name + " " + TypeToLuaAnnotation(ft, worldDesc) + "\n";
        }

        out += ns + " = {}\n";

        // new() constructor — inline table type shows logical (non-flattened) fields.
        out += "\n---@param t? {";
        bool firstField = true;
        for (const auto& [name, field] : desc->GetFields())
        {
            if (field.IsStatic() || field.IsScriptHidden()) continue;
            const TypeDescriptor* ft = field.GetType();
            if (!ft || ft == desc) continue;
            if (!firstField) out += ", ";
            out += name + "?: " + TypeToLuaAnnotation(ft, worldDesc);
            firstField = false;
        }
        out += "}\n";
        out += "---@return " + ns + "\n";
        out += "function " + ns + ".new(t) end\n";

        // Static methods whose namespace matches this type.
        auto it = methodsByNs.find(ns);
        if (it != methodsByNs.end())
            EmitMethodStubs(out, ns, it->second, worldDesc);
    }

    // ── OOP class declarations (from IScriptBindings) ────────────────────────
    for (const auto& ci : classes)
    {
        out += "\n---@class " + ci.Namespace;
        if (!ci.BaseNamespace.empty()) out += " : " + ci.BaseNamespace;
        out += "\n";
        out += "---@field _id integer\n";
        out += ci.Namespace + " = {}\n";

        // Instance methods — emit with colon syntax placeholder (self omitted in params).
        for (const auto& m : ci.Methods)
        {
            auto isValidIdent = [](const std::string& s) {
                if (s.empty()) return false;
                if (!std::isalpha(static_cast<unsigned char>(s[0])) && s[0] != '_') return false;
                return std::all_of(s.begin(), s.end(), [](unsigned char c) {
                    return std::isalnum(c) || c == '_';
                });
            };
            // Params: skip World and the handle (first non-World param).
            std::vector<std::string> pnames;
            bool skippedHandle = false;
            int argIdx = 0;
            for (const auto& p : m.GetDesc().GetParams())
            {
                if (p.Type == worldDesc) continue;
                if (!skippedHandle) { skippedHandle = true; continue; }  // skip self
                pnames.push_back(isValidIdent(p.Name) ? p.Name : ("arg" + std::to_string(argIdx)));
                ++argIdx;
            }
            int annotIdx = 0;
            skippedHandle = false;
            for (const auto& p : m.GetDesc().GetParams())
            {
                if (p.Type == worldDesc) continue;
                if (!skippedHandle) { skippedHandle = true; continue; }  // skip self
                out += "---@param " + pnames[annotIdx] + " " + TypeToLuaAnnotation(p.Type, worldDesc) + "\n";
                ++annotIdx;
            }
            const TypeDescriptor* ret = m.GetDesc().GetReturnType();
            if (ret && ret->GetTypeId() != StaticTypeName<void>::TypeId)
                out += "---@return " + TypeToLuaAnnotation(ret, worldDesc) + "\n";
            out += "function " + ci.Namespace + ":" + m.Name + "(";
            for (size_t i = 0; i < pnames.size(); ++i)
            {
                if (i) out += ", ";
                out += pnames[i];
            }
            out += ") end\n";
        }

        // Static methods — emit with dot syntax.
        auto it = methodsByNs.find(ci.Namespace);
        if (it != methodsByNs.end())
            EmitMethodStubs(out, ci.Namespace, it->second, worldDesc);
    }

    // ── Pure-namespace (feature) method stubs ─────────────────────────────────
    // These are namespaces like Phoenix.Orders that are not struct types or OOP classes.
    for (const auto& [ns, mis] : methodsByNs)
    {
        if (structNamespaces.count(ns) > 0) continue;   // already emitted above
        if (classNamespaces.count(ns) > 0)  continue;   // already emitted above
        out += "\n-- " + ns + "\n";
        EmitMethodStubs(out, ns, mis, worldDesc);
    }

    // ── Utilities ─────────────────────────────────────────────────────────────
    out += "\n---@param s string\n";
    out += "---@return integer\n";
    out += "function Phoenix.Hash(s) end\n";

    return out;
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const char* luaBridgePath = nullptr;
    const char* luaDefsPath   = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--lua-bridge") == 0 && i + 1 < argc)
            luaBridgePath = argv[++i];
        else if (strcmp(argv[i], "--lua-defs") == 0 && i + 1 < argc)
            luaDefsPath = argv[++i];
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s --lua-bridge <path> [--lua-defs <path>]\n", argv[0]);
            return 1;
        }
    }

    if (!luaBridgePath)
    {
        fprintf(stderr, "Usage: %s --lua-bridge <path> [--lua-defs <path>]\n", argv[0]);
        return 1;
    }

    const TypeDescriptor* worldDesc = &TypeRegistry::Get<World>();
    std::vector<MethodInfo>  methods = CollectMethods(worldDesc);
    std::vector<StructInfo>  structs = CollectStructTypes(worldDesc);
    std::vector<ClassInfo>   classes;
    CollectBindings(methods, classes);

    json data = BuildLuaBridgeData(methods, structs, classes, worldDesc);

    try
    {
        inja::Environment env;
        env.set_trim_blocks(true);
        std::string result = env.render(kLuaBridgeTemplate, data);

        std::ofstream out(luaBridgePath);
        if (!out) { fprintf(stderr, "Cannot open '%s'\n", luaBridgePath); return 1; }
        out << result;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Template error: %s\n", e.what());
        return 1;
    }

    if (luaDefsPath)
    {
        std::string defs = BuildLuaDefs(methods, classes, worldDesc);
        std::ofstream out(luaDefsPath);
        if (!out) { fprintf(stderr, "Cannot open '%s'\n", luaDefsPath); return 1; }
        out << defs;
    }

    return 0;
}
