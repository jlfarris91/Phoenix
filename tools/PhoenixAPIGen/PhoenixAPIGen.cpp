// PhoenixAPIGen — dump all reflected types to api.json
//
// Usage:
//   PhoenixAPIGen --output <path/to/api.json>
//
// Output schema (version 1):
//
//   {
//     "version": 1,
//     "types": [
//       {
//         "name":              string,   // canonical C++ type name (FTypeName)
//         "alias":             string,   // short alias if set, else ""
//         "size":              number,   // sizeof(T)
//         "is_interface":      bool,
//         "is_script_hidden":  bool,
//         "is_no_script_table":bool,
//         "namespace":         string,   // from Namespace() metadata, else ""
//         "metadata":          object,   // all key-value metadata pairs
//         "bases":             string[], // all ancestors (direct + inherited)
//         "properties": [
//           {
//             "name":           string,
//             "type":           TypeRef,
//             "is_readonly":    bool,
//             "is_static":      bool,
//             "is_script_hidden": bool,
//             "metadata":       object
//           }
//         ],
//         "methods": [
//           {
//             "name":           string,
//             "is_static":      bool,
//             "is_script_hidden": bool,
//             "metadata":       object,
//             "params":         Param[],
//             "return":         TypeRef
//           }
//         ],
//         "constructors": [ /* same shape as methods */ ]
//       }
//     ]
//   }
//
// TypeRef: { "kind": "void" | primitive-name | "struct", "type"?: string }
// Param:   { "name": string, "type": TypeRef, "is_optional": bool }

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/Reflection/TypeDescriptor.h"
#include "PhoenixSim/Reflection/TypeRegistry.h"

using json = nlohmann::ordered_json;
using namespace Phoenix;

// ── Type → kind string ────────────────────────────────────────────────────────
//
// Maps a TypeDescriptor to a JSON "kind" string matching the old EGenericValueType
// names so api.json consumers see a stable format.

static std::string TypeKind(const TypeDescriptor* type)
{
    if (!type) return "void";

    const FName id = type->GetTypeId();

    if (id == StaticTypeName<void>::TypeId)         return "void";
    if (id == StaticTypeName<bool>::TypeId)         return "bool";
    if (id == StaticTypeName<int8_t>::TypeId)       return "int8";
    if (id == StaticTypeName<uint8_t>::TypeId)      return "uint8";
    if (id == StaticTypeName<int16_t>::TypeId)      return "int16";
    if (id == StaticTypeName<uint16_t>::TypeId)     return "uint16";
    if (id == StaticTypeName<int32_t>::TypeId)      return "int32";
    if (id == StaticTypeName<uint32_t>::TypeId)     return "uint32";
    if (id == StaticTypeName<int64_t>::TypeId)      return "int64";
    if (id == StaticTypeName<uint64_t>::TypeId)     return "uint64";
    if (id == StaticTypeName<float>::TypeId)        return "float";
    if (id == StaticTypeName<double>::TypeId)       return "double";
    if (id == StaticTypeName<std::string>::TypeId)  return "string";
    if (id == StaticTypeName<FName>::TypeId)        return "name";
    if (type->IsTemplate("Phoenix::TFixed"))        return "fixed_point";

    return "struct";
}

// ── Serialization helpers ─────────────────────────────────────────────────────

static json SerializeTypeRef(const TypeDescriptor* type)
{
    json j;
    const std::string kind = TypeKind(type);
    j["kind"] = kind;
    if (kind == "struct" && type)
        j["type"] = type->GetName();
    return j;
}

static json SerializeParam(const ParamDescriptor& param)
{
    json j;
    j["name"]        = param.Name;
    j["type"]        = SerializeTypeRef(param.Type);
    j["is_optional"] = param.IsOptional;
    return j;
}

static json SerializeMetadata(const std::unordered_map<std::string, std::string>& metadata)
{
    json j = json::object();
    std::vector<std::string> keys;
    keys.reserve(metadata.size());
    for (auto& [k, _] : metadata) keys.push_back(k);
    std::sort(keys.begin(), keys.end());
    for (auto& k : keys) j[k] = metadata.at(k);
    return j;
}

static json SerializeMethod(const MethodDescriptor& method)
{
    json j;
    j["name"]             = method.GetName();
    j["is_static"]        = method.IsStatic();
    j["is_script_hidden"] = method.IsScriptHidden();
    j["metadata"]         = SerializeMetadata(method.GetMetadata());

    json params = json::array();
    for (auto& param : method.GetParams())
        params.push_back(SerializeParam(param));
    j["params"] = params;

    j["return"] = SerializeTypeRef(method.GetReturnType());
    return j;
}

static json SerializeProperty(const PropertyDescriptor& prop)
{
    json j;
    j["name"]             = prop.GetName();
    j["type"]             = SerializeTypeRef(prop.GetType());
    j["is_readonly"]      = prop.IsReadOnly();
    j["is_static"]        = prop.IsStatic();
    j["is_script_hidden"] = prop.IsScriptHidden();
    j["metadata"]         = SerializeMetadata(prop.GetMetadata());
    return j;
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    std::string outputPath;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--output" && i + 1 < argc)
            outputPath = argv[++i];
    }

    if (outputPath.empty())
    {
        std::cerr << "Usage: PhoenixAPIGen --output <path/to/api.json>\n";
        return 1;
    }

    const auto& allTypes = TypeRegistry::GetAll();

    std::vector<const TypeDescriptor*> sorted;
    sorted.reserve(allTypes.size());
    for (auto& [hash, td] : allTypes) sorted.push_back(td.get());
    std::sort(sorted.begin(), sorted.end(), [](const TypeDescriptor* a, const TypeDescriptor* b) {
        return a->GetName() < b->GetName();
    });

    json typesArray = json::array();

    for (const TypeDescriptor* td : sorted)
    {
        json typeObj;
        typeObj["name"]               = td->GetName();
        typeObj["alias"]              = td->GetAlias();
        typeObj["size"]               = td->GetSize();
        typeObj["is_interface"]       = td->IsInterface();
        typeObj["is_script_hidden"]   = td->IsScriptHidden();

        {
            const auto& meta = td->GetMetadata();
            auto nsIt = meta.find("Namespace");
            typeObj["namespace"] = (nsIt != meta.end()) ? nsIt->second : "";
        }

        typeObj["metadata"] = SerializeMetadata(td->GetMetadata());

        {
            json bases = json::array();
            for (const FName& baseId : td->GetBaseTypeIds())
            {
                const TypeDescriptor* base = TypeRegistry::Get(baseId);
                if (base) bases.push_back(base->GetName());
            }
            typeObj["bases"] = bases;
        }

        {
            std::vector<const PropertyDescriptor*> props;
            props.reserve(td->GetProperties().size());
            for (auto& [n, p] : td->GetProperties()) props.push_back(&p);
            std::sort(props.begin(), props.end(), [](const PropertyDescriptor* a, const PropertyDescriptor* b) {
                return a->GetName() < b->GetName();
            });
            json propsArray = json::array();
            for (auto* p : props) propsArray.push_back(SerializeProperty(*p));
            typeObj["properties"] = propsArray;
        }

        {
            std::vector<const MethodDescriptor*> methods;
            methods.reserve(td->GetMethods().size());
            for (auto& [n, m] : td->GetMethods()) methods.push_back(&m);
            std::sort(methods.begin(), methods.end(), [](const MethodDescriptor* a, const MethodDescriptor* b) {
                return a->GetName() < b->GetName();
            });
            json methodsArray = json::array();
            for (auto* m : methods) methodsArray.push_back(SerializeMethod(*m));
            typeObj["methods"] = methodsArray;
        }

        {
            json ctorsArray = json::array();
            for (auto& ctor : td->GetConstructors())
                ctorsArray.push_back(SerializeMethod(ctor));
            typeObj["constructors"] = ctorsArray;
        }

        typesArray.push_back(typeObj);
    }

    json output;
    output["version"] = 1;
    output["types"]   = typesArray;

    std::ofstream file(outputPath);
    if (!file.is_open())
    {
        std::cerr << "PhoenixAPIGen: failed to open output file: " << outputPath << "\n";
        return 1;
    }

    file << output.dump(2) << "\n";
    std::cout << "PhoenixAPIGen: wrote " << sorted.size() << " types to " << outputPath << "\n";
    return 0;
}
