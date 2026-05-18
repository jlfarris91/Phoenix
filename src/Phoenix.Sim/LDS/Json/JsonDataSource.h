#pragma once

#include <nlohmann/json.hpp>

#include "Phoenix.Sim/Logging.h"
#include "Phoenix.Sim/Platform.h"

namespace Phoenix::LDS::Json
{
    class JsonDataSource : ILogCollectionOwner<LogMessage>
    {
    public:

        // Loads a JsonDataSource from the specified catalog file.
        static std::shared_ptr<JsonDataSource> LoadFromCatalog(const std::string& catalogPath);

        // Gets or sets the parent data source.
        // If a type or object is not found in this data source, the parent data source will be queried.
        std::shared_ptr<JsonDataSource> GetParent() const;

        // Sets the parent data source.
        // If a type or object is not found in this data source, the parent data source will be queried.
        void SetParent(const std::shared_ptr<JsonDataSource>& parent);

        // Registers a type with this data source.
        bool RegisterType(const nlohmann::json& typeJson);

        // Gets a map of all types registered with this data source.
        const std::unordered_map<std::string, nlohmann::json>& GetRegisteredTypes() const;

        // Finds a type in this data source by its ID.
        // If the type is not found in this data source, the parent data source will be queried.
        const nlohmann::json* FindType(const std::string& typeId) const;

        void RegisterInterface(const std::string& interfaceId, const std::string& typeId);

        const std::vector<std::string>& GetInterfacesOfType(const std::string& typeId) const;
        const std::vector<std::string>& GetTypesImplementingInterface(const std::string& interfaceId) const;

        // Returns true if the data source contains a given type or interface id.
        bool HasTypeOrInterface(const std::string& typeOrInterfaceId) const;

        // Registers an object with this data source.
        bool RegisterObject(const nlohmann::json& objectJson);

        // Gets a map of all objects registered with this data source.
        const std::unordered_map<std::string, nlohmann::json>& GetRegisteredObjects() const;

        // Finds an object in this data source by its ID.
        // If the object is not found in this data source, the parent data source will be queried.
        const nlohmann::json* FindObject(const std::string& objectId) const;

    private:
        std::shared_ptr<JsonDataSource> Parent;
        std::unordered_map<std::string, nlohmann::json> Types;
        std::unordered_map<std::string, nlohmann::json> Objects;
        std::unordered_map<std::string, std::vector<std::string>> TypeIdToInterfaceIds;
        std::unordered_map<std::string, std::vector<std::string>> InterfaceToTypeIds;
    };
}
