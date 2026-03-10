#include "PhoenixSim/LDS/Json/JsonDataSource.h"

#include <fstream>

using namespace Phoenix;
using namespace Phoenix::LDS::Json;
namespace fs = std::filesystem;

std::shared_ptr<JsonDataSource> JsonDataSource::LoadFromCatalog(const std::string& catalogPath)
{
    auto dataSource = std::make_shared<JsonDataSource>();

    fs::path absoluteCatalogPath = fs::absolute(catalogPath);
    fs::path absoluteCatalogDir = absoluteCatalogPath.parent_path();

    std::ifstream catalogStream(absoluteCatalogPath);
    if (!catalogStream.is_open())
    {
        dataSource->LogError("Failed to open Catalog file '{}'", catalogPath);
        return dataSource;
    }

    nlohmann::json catalogJson;

    try
    {
        catalogJson = nlohmann::json::parse(catalogStream, nullptr, false);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        dataSource->LogError("Failed to parse Catalog file '{}': {}", catalogPath, ex.what());
        return dataSource;
    }

    auto typesIter = catalogJson.find("types");
    if (typesIter != catalogJson.end())
    {
        for (auto&& typesPath : typesIter->items())
        {
            auto typeDir = absolute(absoluteCatalogDir / typesPath.value().get<std::string>());
            for (const auto& filePath : fs::recursive_directory_iterator(typeDir))
            {
                if (!filePath.is_regular_file())
                {
                    continue;
                }

                std::ifstream fileStream(filePath.path());
                if (!fileStream.is_open())
                {
                    dataSource->LogError("Failed to open Type file '{}'", filePath.path().generic_string());
                    continue;
                }

                nlohmann::json typeJson;

                try
                {
                    typeJson = nlohmann::json::parse(fileStream, nullptr, false);
                }
                catch (const nlohmann::detail::exception& ex)
                {
                    dataSource->LogError("Failed to parse Type file '{}': {}", filePath.path().generic_string(), ex.what());
                    return dataSource;
                }

                dataSource->RegisterType(typeJson);
            }
        }
    }

    auto objectsIter = catalogJson.find("objects");
    if (objectsIter != catalogJson.end())
    {
        for (auto&& objectsPath : objectsIter->items())
        {
            auto objectsDir = absolute(absoluteCatalogDir / objectsPath.value().get<std::string>());
            for (const auto& filePath : fs::recursive_directory_iterator(objectsDir))
            {
                if (!filePath.is_regular_file())
                {
                    continue;
                }

                std::ifstream fileStream(filePath.path());
                if (!fileStream.is_open())
                {
                    dataSource->LogError("Failed to open Object file '{}'", filePath.path().generic_string());
                    continue;
                }

                nlohmann::json objectJson;

                try
                {
                    objectJson = nlohmann::json::parse(fileStream, nullptr, false);
                }
                catch (const nlohmann::detail::exception& ex)
                {
                    dataSource->LogError("Failed to parse Object file '{}': {}", filePath.path().generic_string(), ex.what());
                    return dataSource;
                }

                dataSource->RegisterObject(objectJson);
            }
        }
    }

    return dataSource;
}

std::shared_ptr<JsonDataSource> JsonDataSource::GetParent() const
{
    return Parent;
}

void JsonDataSource::SetParent(const std::shared_ptr<JsonDataSource>& parent)
{
    Parent = parent;
}

bool JsonDataSource::RegisterType(const nlohmann::json& typeJson)
{
    auto idIter = typeJson.find("id");
    if (idIter == typeJson.end())
    {
        this->LogError("Type is missing required 'id' property.");
        return false;
    }

    std::string typeId = idIter->get<std::string>();
    if (Types.contains(typeId))
    {
        this->LogError("Type with id '{}' has already been registered.", typeId);
        return false;
    }

    Types.emplace(typeId, typeJson);

    auto implementsIter = typeJson.find("implements");
    if (implementsIter != typeJson.end())
    {
        const nlohmann::json& implementsArray = *implementsIter;
        for (const auto& implementIdJson : implementsArray)
        {
            std::string implementId = implementIdJson.get<std::string>();

            TypeIdToInterfaceIds[typeId].push_back(implementId);
            InterfaceToTypeIds[implementId].push_back(typeId);
        }
    }

    return true;
}

const std::unordered_map<std::string, nlohmann::basic_json<>>& JsonDataSource::GetRegisteredTypes() const
{
    return Types;
}

const nlohmann::json* JsonDataSource::FindType(const std::string& typeId) const
{
    auto iter = Types.find(typeId);
    if (iter != Types.end())
    {
        return &iter->second;
    }
    if (Parent)
    {
        return Parent->FindType(typeId);
    }
    return nullptr;
}

void JsonDataSource::RegisterInterface(const std::string& interfaceId, const std::string& typeId)
{
    InterfaceToTypeIds[interfaceId].push_back(typeId);
}

const std::vector<std::string>& JsonDataSource::GetInterfacesOfType(const std::string& typeId) const
{
    static std::vector<std::string> EmptyArray;
    auto iter = TypeIdToInterfaceIds.find(typeId);
    return iter != TypeIdToInterfaceIds.end() ? iter->second : EmptyArray;
}

const std::vector<std::string>& JsonDataSource::GetTypesImplementingInterface(const std::string& interfaceId) const
{
    static std::vector<std::string> EmptyArray;
    auto iter = InterfaceToTypeIds.find(interfaceId);
    return iter != InterfaceToTypeIds.end() ? iter->second : EmptyArray;
}

bool JsonDataSource::HasTypeOrInterface(const std::string& typeOrInterfaceId) const
{
    return FindType(typeOrInterfaceId) || InterfaceToTypeIds.contains(typeOrInterfaceId);
}

const nlohmann::json* JsonDataSource::FindObject(const std::string& objectId) const
{
    auto iter = Objects.find(objectId);
    if (iter != Objects.end())
    {
        return &iter->second;
    }
    if (Parent)
    {
        return Parent->FindObject(objectId);
    }
    return nullptr;
}

bool JsonDataSource::RegisterObject(const nlohmann::json& objectJson)
{
    auto idIter = objectJson.find("id");
    if (idIter == objectJson.end())
    {
        this->LogError("Object is missing required 'id' property.");
        return false;
    }

    std::string rootObjectId = idIter->get<std::string>();
    if (Objects.contains(rootObjectId))
    {
        this->LogError("Object with id '{}' has already been registered.", rootObjectId);
        return false;
    }

    Objects.emplace(rootObjectId, objectJson);
    return true;
}

const std::unordered_map<std::string, nlohmann::basic_json<>>& JsonDataSource::GetRegisteredObjects() const
{
    return Objects;
}
