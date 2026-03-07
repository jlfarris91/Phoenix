
#pragma once

#include "PhoenixSim/Containers/Optional.h"
#include "PhoenixSim/LDS/Json/JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    struct PHOENIX_SIM_API JsonCatalogObjectBuilder : JsonCatalogBuilderBase
    {
        using json = nlohmann::json;

        JsonCatalogObjectBuilder(const JsonDataSource* dataSource, HeapLDSCatalog* catalog);

        bool RegisterAllObjects();

        bool RegisterObject(const json& objectJson);

        bool ProcessJsonObject(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath);

        bool ProcessObjectProperties(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath);

        bool ProcessObjectRef(const PHXString& rootObjectId, const json& json, const PHXString& jsonPath);

        bool ProcessArray(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath);

        bool ProcessEnum(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath);

        bool ProcessEnumFlags(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath);

        bool ProcessValueProperty(const PHXString& rootObjectId, const json& json, const PHXString& path);

        const LDSRecord* FindTypeRecordForObject(const FName& objectId, const FName& propertyId);

        template <class ...TArgs>
        void EmplaceObjectRecord(const PHXString& rootObjectId, const PHXString& path, TArgs&&... args)
        {
            PHXString finalPath = path;
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
        TOptional<PHXString> PathPostFix;
        ELDSCatalogRecordStore RecordStore = ELDSCatalogRecordStore::Object;
    };
}