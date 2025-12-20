
#pragma once

#include <nlohmann/json.hpp>

#include "PhoenixSim/Logging.h"
#include "PhoenixSim/Platform.h"

namespace Phoenix::LDS::Json
{
    class JsonDataSource : ILogger<LogMessage>
    {
    public:

        // Loads a JsonDataSource from the specified catalog file.
        static TSharedPtr<JsonDataSource> LoadFromCatalog(const PHXString& catalogPath);

        // Gets or sets the parent data source.
        // If a type or object is not found in this data source, the parent data source will be queried.
        TSharedPtr<JsonDataSource> GetParent() const;

        // Sets the parent data source.
        // If a type or object is not found in this data source, the parent data source will be queried.
        void SetParent(const TSharedPtr<JsonDataSource>& parent);

        // Registers a type with this data source.
        bool RegisterType(const nlohmann::json& typeJson);

        // Gets a map of all types registered with this data source.
        const TMap<PHXString, nlohmann::json>& GetRegisteredTypes() const;

        // Finds a type in this data source by its ID.
        // If the type is not found in this data source, the parent data source will be queried.
        const nlohmann::json* FindType(const PHXString& typeId) const;

        void RegisterInterface(const PHXString& interfaceId, const PHXString& typeId);

        const TArray<PHXString>& GetInterfacesOfType(const PHXString& typeId) const;
        const TArray<PHXString>& GetTypesImplementingInterface(const PHXString& interfaceId) const;

        // Returns true if the data source contains a given type or interface id.
        bool HasTypeOrInterface(const PHXString& typeOrInterfaceId) const;

        // Registers an object with this data source.
        bool RegisterObject(const nlohmann::json& objectJson);

        // Gets a map of all objects registered with this data source.
        const TMap<PHXString, nlohmann::json>& GetRegisteredObjects() const;

        // Finds an object in this data source by its ID.
        // If the object is not found in this data source, the parent data source will be queried.
        const nlohmann::json* FindObject(const PHXString& objectId) const;

    private:
        TSharedPtr<JsonDataSource> Parent;
        TMap<PHXString, nlohmann::json> Types;
        TMap<PHXString, nlohmann::json> Objects;
        TMap<PHXString, TArray<PHXString>> TypeIdToInterfaceIds;
        TMap<PHXString, TArray<PHXString>> InterfaceToTypeIds;
    };
}
