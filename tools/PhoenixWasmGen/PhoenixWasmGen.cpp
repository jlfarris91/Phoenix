/*
 * PhoenixWasmGen.cpp — WASM host-API header generator.
 *
 * Usage:
 *   PhoenixWasmGen --host-api <out/host_api.h>
 *
 * Output:
 *   host_api.h — Language-agnostic C header with one
 *                __attribute__((import_module/import_name)) extern declaration
 *                per script-visible static method.  Any language compiled to
 *                WASM (C, Rust, AssemblyScript, …) can include this header to
 *                call Phoenix host functions.
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
#include <string>
#include <vector>

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "PhoenixScript/WasmUtility.h"
#include "PhoenixSim/Reflection/Variant.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"
#include "PhoenixSim/Scripting/IScriptBindings.h"
#include "PhoenixSim/Scripting/ScriptModuleBuilder.h"
#include "PhoenixSim/Worlds.h"

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

// "Phoenix.Unit" + "SpawnUnit" → "phx_Phoenix_Unit_SpawnUnit"
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

// ── Struct helpers ────────────────────────────────────────────────────────────

struct StructField
{
    std::string Name;
    char        WasmChar;
    size_t      ByteOffset;
    int         FractionalBits;
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
            result.push_back({ qualName, c, offset, fracBits });
            offset += sz;
        }
    }
    return result;
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

    const MethodDescriptor& GetDesc() const { return OwnedDesc ? *OwnedDesc : *Desc; }

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

// ── inja template ─────────────────────────────────────────────────────────────

static constexpr const char* kHostApiTemplate = R"TMPL(/* Auto-generated by PhoenixWasmGen — do not edit.
 * Regenerate: build the PhoenixWasmGen target.
 *
 * Calling convention (matches WasmEnvironment::BuildWasmSignature):
 *   • WorldRef params are injected by the host — NOT in the WASM signature.
 *   • Struct params (≤32 bytes) are expanded to scalar fields (alphabetical order).
 *   • Struct returns use sret convention: void return + i32 sret as first param.
 *   • i32 → int32_t,  i64 → int64_t,  f32 → float,  f64 → double
 */

#pragma once
#include <stdint.h>
{% for ns in namespaces %}

/* ── {{ ns.name }} ── */
{% for m in ns.methods %}

{% if m.injected != "" %}/* {{ m.name }}: {{ m.sig }}  [{{ m.injected }}] */
{% else %}/* {{ m.name }}: {{ m.sig }} */
{% endif %}__attribute__((import_module("{{ ns.name }}"), import_name("{{ m.name }}")))
extern {{ m.ret }} {{ m.import_name }}({{ m.params }});
{% endfor %}
{% endfor %}
)TMPL";

// ── data model ────────────────────────────────────────────────────────────────

static json BuildHostApiData(const std::vector<MethodInfo>& methods,
                              const TypeDescriptor* worldDesc)
{
    json data;
    data["namespaces"] = json::array();

    std::string lastNs;

    for (const auto& mi : methods)
    {
        if (mi.Namespace != lastNs)
        {
            data["namespaces"].push_back({ {"name", mi.Namespace}, {"methods", json::array()} });
            lastNs = mi.Namespace;
        }
        json& curNs = data["namespaces"].back();

        const TypeDescriptor& returnType = *mi.GetDesc().GetReturnType();
        const bool retIsStruct = Script::IsExpandableStruct(returnType);
        std::vector<std::tuple<char, std::string>> paramChars;
        std::string notes;

        if (retIsStruct) paramChars.emplace_back('i', "");

        for (const auto& p : mi.GetDesc().GetParams())
        {
            if (p.Type == worldDesc)
            {
                notes += "World (Skipped), ";
                continue;
            }

            if (Script::IsExpandableStruct(*p.Type))
            {
                for (const auto& f : FlattenStructFields(*p.Type))
                {
                    paramChars.emplace_back(f.WasmChar, std::format("{}_{}", p.Name, f.Name));
                }
                notes += std::format("{} (Expanded), ", p.Type->GetAliasOrName());
            }
            else
            {
                notes += std::format("{}, ", p.Type->GetAliasOrName());
                char c = Script::ToWasmTypeChar(*p.Type);
                if (c != 'v')
                {
                    paramChars.emplace_back(c, p.Name);
                }
                else
                {
                    notes += "Unknown ";
                }
            }
        }

        if (!notes.empty() && notes.back() == ' ')
        {
            notes.pop_back();
            if (notes.back() == ',')
            {
                notes.pop_back();
            }
        }

        char retChar = retIsStruct ? 'v' : Script::ToWasmTypeChar(returnType);
        std::string retC = (retChar == 'v' || !CType(retChar)) ? "void" : std::string(CType(retChar));

        std::string sig;
        sig += retChar; sig += '(';
        for (auto && [c, name] : paramChars)
        {
            sig += c;
        }
        sig += ')';

        std::string params;
        for (size_t i = 0; i < paramChars.size(); ++i)
        {
            if (i) params += ", ";
            params += CType(std::get<0>(paramChars[i]));
            params += " ";
            params += std::get<1>(paramChars[i]);
        }

        if (params.empty())
        {
            params = "void";
        }

        curNs["methods"].push_back({
            {"name",        mi.Name},
            {"sig",         sig},
            {"injected",    notes},
            {"ret",         retC},
            {"import_name", ImportFuncName(mi.Namespace, mi.Name)},
            {"params",      params}
        });
    }

    return data;
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    const char* hostApiPath = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--host-api") == 0 && i + 1 < argc)
            hostApiPath = argv[++i];
        else
        {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s --host-api <path>\n", argv[0]);
            return 1;
        }
    }

    if (!hostApiPath)
    {
        fprintf(stderr, "Usage: %s --host-api <path>\n", argv[0]);
        return 1;
    }

    const TypeDescriptor* worldDesc = &TypeRegistry::Get<World>();
    std::vector<MethodInfo> methods = CollectMethods(worldDesc);

    // ── IScriptBindings ───────────────────────────────────────────────────────
    // Flatten all IScriptBindings class methods and free functions into the
    // methods list.  Class instance methods and statics both become plain host
    // imports — the handle is just another WASM i32 argument.
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
        // Insert or replace: IScriptBindings version wins over reflection for the same (ns, name).
        auto appendMethod = [&](const std::string& ns, const MethodDescriptor& m)
        {
            MethodInfo mi;
            mi.Namespace  = ns;
            mi.OwnedDesc  = m;
            mi.Desc       = &*mi.OwnedDesc;
            mi.Name       = mi.GetDesc().GetName();
            auto it = std::find_if(methods.begin(), methods.end(),
                [&](const MethodInfo& e) { return e.Namespace == mi.Namespace && e.Name == mi.Name; });
            if (it != methods.end()) *it = std::move(mi);
            else methods.push_back(std::move(mi));
        };
        for (const auto& entry : builder.GetFunctions())
            appendMethod(entry.Namespace, entry.Method);
        for (const auto& cls : builder.GetClasses())
        {
            for (const auto& m : cls.Methods)  appendMethod(cls.Namespace, m);
            for (const auto& s : cls.Statics)  appendMethod(cls.Namespace, s);
        }
        std::sort(methods.begin(), methods.end(), [](const MethodInfo& a, const MethodInfo& b)
        { return a.Namespace < b.Namespace || (a.Namespace == b.Namespace && a.Name < b.Name); });
    }

    json data = BuildHostApiData(methods, worldDesc);

    try
    {
        inja::Environment env;
        env.set_trim_blocks(true);
        std::string result = env.render(kHostApiTemplate, data);

        std::ofstream out(hostApiPath);
        if (!out) { fprintf(stderr, "Cannot open '%s'\n", hostApiPath); return 1; }
        out << result;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Template error: %s\n", e.what());
        return 1;
    }

    return 0;
}
