#include "WasmRuntime.h"

#include <fstream>
#include <vector>

#include "Phoenix.Sim/Logging.h"
#include "Phoenix.Sim/Session.h"
#include "Phoenix.Sim/Scripting/IScriptBindings.h"
#include "Phoenix.Sim/Scripting/ScriptModuleBuilder.h"

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

const std::string& WasmRuntime::GetScriptPath() const
{ 
    return ScriptPath; 
}

const std::vector<uint8>& WasmRuntime::GetWasmBytes() const
{ 
    return WasmBytes; 
}

const std::vector<WasmHostEntry>& WasmRuntime::GetRegistrations() const
{ 
    return Registrations; 
}

void WasmRuntime::RegisterType(const TypeDescriptor& desc)
{
    // Skip types with nothing script-visible.
    if (desc.IsScriptHidden())
    {
        return;
    }

    const bool isValueType = (desc.GetSize() > 0 && desc.GetSize() <= 32);
    const bool hasFields   = !desc.GetFields().empty() || !desc.GetProperties().empty();

    bool hasStaticMethods = false;
    for (const auto& methodDescriptor : desc.GetMethods() | std::views::values)
    {
        if (methodDescriptor.IsStatic() && !methodDescriptor.IsScriptHidden())
        {
            hasStaticMethods = true;
            break;
        }
    }

    if (!hasStaticMethods && !(isValueType && hasFields))
    {
        return;
    }

    const std::string ns = desc.GetScriptNamespace();

    // ── Static methods ────────────────────────────────────────────────────────
    for (const auto& method : desc.GetMethods() | std::views::values)
    {
        if (!method.IsStatic()) continue;
        if (method.IsScriptHidden()) continue;
        Registrations.push_back({ ns, method });
    }
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

    ScriptPath = scriptPath.generic_string();

    Registrations.clear();
    for (const auto& desc : TypeRegistry::GetAll() | std::views::values)
    {
        RegisterType(*desc);
    }

    // ── IScriptBindings ───────────────────────────────────────────────────────
    //
    // Discover all concrete IScriptBindings implementations, instantiate each
    // one, call Describe(), then flatten the resulting ScriptFunctionEntries and
    // ScriptClassEntry methods into the Registrations list.
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
        for (const auto& entry : builder.GetFunctions())
            Registrations.push_back({ entry.Namespace, entry.Method });
        for (const auto& cls : builder.GetClasses())
        {
            for (const auto& m : cls.Methods)
                Registrations.push_back({ cls.Namespace, m });
            for (const auto& s : cls.Statics)
                Registrations.push_back({ cls.Namespace, s });
        }
    }

    return true;
}
