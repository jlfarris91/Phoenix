#include "WasmRuntime.h"

#include <fstream>
#include <vector>

#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Session.h"
#include "PhoenixSim/Scripting/IScriptBindings.h"

using namespace Phoenix;

namespace
{
    bool LoadBytesFromFile(const char* path, std::vector<uint8_t>& outBytes)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            LogError("[PhoenixScript] Cannot open '{}'", path);
            return false;
        }
        const auto size = static_cast<std::streamsize>(file.tellg());
        file.seekg(0);
        outBytes.resize(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(outBytes.data()), size);
        return file.good();
    }
}

WasmRuntime::WasmRuntime(const std::shared_ptr<Phoenix::Session>& session)
    : Session(session)
{
}

void WasmRuntime::RegisterType(const TypeDescriptor& desc)
{
    // Skip types with nothing script-visible.
    if (desc.IsScriptHidden()) return;

    const bool isValueType = (desc.GetSize() > 0 && desc.GetSize() <= 32);
    const bool hasFields   = !desc.GetProperties().empty();

    bool hasStaticMethods = false;
    for (const auto& [n, m] : desc.GetMethods())
        if (m.IsStatic() && !HasAnyFlags(m.Flags, EMemberDescriptorFlags::ScriptHidden))
            { hasStaticMethods = true; break; }

    if (!hasStaticMethods && !(isValueType && hasFields)) return;

    // Explicit "Namespace" metadata takes priority; otherwise fall back to the
    // type's qualified C++ name converted to a dot-separated Lua path.
    std::string ns;
    {
        const auto it = desc.GetMetadata().find("Namespace");
        if (it != desc.GetMetadata().end())
            ns = it->second;
        else
        {
            ns = desc.GetQualifiedCName();
            for (size_t i = 0; i + 1 < ns.size(); )
            {
                if (ns[i] == ':' && ns[i+1] == ':') { ns.replace(i, 2, "."); }
                else ++i;
            }
        }
    }

    // ── Static methods ────────────────────────────────────────────────────────
    for (const auto& [n, method] : desc.GetMethods())
    {
        if (!method.IsStatic()) continue;
        if (HasAnyFlags(method.Flags, EMemberDescriptorFlags::ScriptHidden)) continue;
        Registrations.push_back({ .ImportModule = ns, .Descriptor = method });
    }

    // ── Synthesized field getter / setter for value types (≤32 bytes) ─────────
    //
    // Each field becomes two static host functions:
    //   get_<field>(self: T) -> fieldType          — reads one field from the struct
    //   set_<field>(self: T, value: fieldType) -> T — returns a modified copy
    //
    // The accessor's Execute lambda reconstructs the struct from args[0].Buffer,
    // reads/writes the field, and returns the result.
    if (isValueType && hasFields)
    {
        for (const auto& [propName, prop] : desc.GetProperties())
        {
            if (HasAnyFlags(prop.Flags, EMemberDescriptorFlags::ScriptHidden)) continue;
            if (!prop.PropertyAccessor) continue;

            const GenericValueTypeRef selfType  { EGenericValueType::Unknown, &desc };
            const GenericValueTypeRef fieldType = prop.GetTypeRef();

            // Getter.
            {
                auto accessor = prop.PropertyAccessor;
                WasmHostEntry entry;
                entry.ImportModule          = ns;
                entry.Descriptor.Name       = "get_" + propName;
                entry.Descriptor.Params.push_back({ "self",  selfType  });
                entry.Descriptor.Return.Type = fieldType;
                entry.Descriptor.Function.HasSelfParam = false;
                entry.Descriptor.Function.Invoke =
                    [accessor, fieldType](void*, std::span<const GenericValue> args) -> GenericValue
                    {
                        if (args.empty()) return GenericValue::Void();
                        GenericValue result;
                        result.Type = fieldType;
                        accessor->Get(args[0].Buffer, result.Buffer, sizeof(result.Buffer));
                        return result;
                    };
                Registrations.push_back(std::move(entry));
            }

            // Setter (skip if accessor is read-only).
            if (!prop.PropertyAccessor->IsReadOnly())
            {
                auto accessor = prop.PropertyAccessor;
                WasmHostEntry entry;
                entry.ImportModule           = ns;
                entry.Descriptor.Name        = "set_" + propName;
                entry.Descriptor.Params.push_back({ "self",  selfType  });
                entry.Descriptor.Params.push_back({ "value", fieldType });
                entry.Descriptor.Return.Type  = selfType;  // returns modified copy
                entry.Descriptor.Function.HasSelfParam = false;
                entry.Descriptor.Function.Invoke =
                    [accessor, selfType](void*, std::span<const GenericValue> args) -> GenericValue
                    {
                        if (args.size() < 2) return GenericValue::Void();
                        GenericValue result = args[0];  // copy the struct
                        result.Type = selfType;
                        accessor->Set(result.Buffer, args[1].Buffer, sizeof(args[1].Buffer));
                        return result;
                    };
                Registrations.push_back(std::move(entry));
            }
        }
    }
}

void WasmRuntime::OpenNamespace(const char* ns)
{
    CurrentNamespace = ns ? ns : "";
}

void WasmRuntime::CloseNamespace()
{
    CurrentNamespace.clear();
}

void WasmRuntime::RegisterFunction(const MethodDescriptor& fn)
{
    Registrations.push_back({.ImportModule = CurrentNamespace, .Descriptor = fn });
}

bool WasmRuntime::LoadFile(const char* path)
{
    if (!ScriptPath.empty())
    {
        return false;
    }

    namespace fs = std::filesystem;
    fs::path scriptPath = path;
    if (!fs::exists(scriptPath))
    {
        scriptPath = fs::absolute(fs::path(Session->GetDataDirectory()) / path);
    }

    if (!LoadBytesFromFile(scriptPath.generic_string().c_str(), WasmBytes))
    {
        return false;
    }

    Registrations.clear();
    ScriptPath = scriptPath.generic_string();

    for (const auto& desc : TypeRegistry::GetAll() | std::views::values)
    {
        RegisterType(*desc);
    }

    std::vector<std::shared_ptr<IScriptBindings>> bindingServices;
    Session->GetServices2<IScriptBindings>(bindingServices);

    for (const auto& svc : bindingServices)
    {
        auto bindings = static_pointer_cast<IScriptBindings>(svc);
        OpenNamespace(bindings->GetNamespace());
        bindings->Register(*this);
        CloseNamespace();
    }

    return true;
}

bool WasmRuntime::ExecString(const std::string&)
{
    // WASM runtimes do not support evaluating source strings.
    return false;
}

void WasmRuntime::SetCurrentWorld(World*)
{
}

void WasmRuntime::OnBindingsComplete()
{
}