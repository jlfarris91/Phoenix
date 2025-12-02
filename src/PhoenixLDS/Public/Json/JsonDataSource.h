
#pragma once

#include <nlohmann/json.hpp>

#include "Platform.h"

namespace Phoenix::LDS::Json
{
    class JsonDataSource
    {
    public:

        // Loads a JsonDataSource from the specified directory.
        static TSharedPtr<JsonDataSource> LoadFromDirectory(const PHXString& directoryPath);

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

        // Registers an object with this data source.
        bool RegisterObject(const nlohmann::json& objectJson);

        // Gets a map of all objects registered with this data source.
        const TMap<PHXString, nlohmann::json>& GetRegisteredObjects() const;

        // Finds an object in this data source by its ID.
        // If the object is not found in this data source, the parent data source will be queried.
        const nlohmann::json* FindObject(const PHXString& objectId) const;

    private:
        TSharedPtr<JsonDataSource> Parent;
        TMap<PHXString, nlohmann::json> Objects;
        TMap<PHXString, nlohmann::json> Types;
    };
}
