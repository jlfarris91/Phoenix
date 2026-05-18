#pragma once

#include "Phoenix.Sim/Containers/Optional.h"
#include "Phoenix.Sim.LDS/Json/JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    struct PHOENIX_SIM_API JsonCatalogObjectBuilder : JsonCatalogBuilderBase
    {
        using json = nlohmann::json;

        JsonCatalogObjectBuilder(const JsonDataSource* dataSource, HeapLDSCatalog* catalog);

        bool RegisterAllObjects();

        bool RegisterObject(const json& objectJson);

        bool ProcessJsonObject(
            const std::string& rootObjectId,
            const json& json,
            const std::string& jsonPath,
            const std::string& typePath);

        bool ProcessObjectProperties(
            const std::string& rootObjectId,
            const json& json,
            const std::string& jsonPath,
            const std::string& typePath);

        bool ProcessObjectRef(const std::string& rootObjectId, const json& json, const std::string& jsonPath);

        bool ProcessArray(
            const std::string& rootObjectId,
            const json& json,
            const std::string& jsonPath,
            const std::string& typePath);

        bool ProcessEnum(
            const std::string& rootObjectId,
            const json& json,
            const std::string& jsonPath,
            const std::string& typePath);

        bool ProcessEnumFlags(
            const std::string& rootObjectId,
            const json& json,
            const std::string& jsonPath,
            const std::string& typePath);

        bool ProcessValueProperty(const std::string& rootObjectId, const json& json, const std::string& path);

        const LDSRecord* FindTypeRecordForObject(const FName& objectId, const FName& propertyId);

        template <class ...TArgs>
        void EmplaceObjectRecord(const std::string& rootObjectId, const std::string& path, TArgs&&... args)
        {
            std::string finalPath = path;
            if (PathPostFix.IsSet())
            {
                finalPath += PathPostFix.Get();
            }

            if (RecordStore == ELDSCatalogRecordStore::Type)
            {
                Catalog->EmplaceTypeRecord(rootObjectId, finalPath, std::forward<TArgs>(args)...);
            }
            else
            {
                Catalog->EmplaceObjectRecord(rootObjectId, finalPath, std::forward<TArgs>(args)...);
            }
        }

        TOptional<FName> TypeIdOverride;
        TOptional<std::string> PathPostFix;
        ELDSCatalogRecordStore RecordStore = ELDSCatalogRecordStore::Object;
    };
}