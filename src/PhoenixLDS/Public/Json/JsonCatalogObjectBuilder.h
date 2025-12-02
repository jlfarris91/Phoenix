
#pragma once

#include "JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
    PHOENIX_LDS_API struct JsonCatalogObjectBuilder : JsonCatalogBuilderBase<TCatalog>
    {
        using json = nlohmann::json;

        JsonCatalogObjectBuilder(const JsonDataSource* dataSource, TCatalog* catalog)
            : JsonCatalogBuilderBase<TCatalog>(dataSource, catalog)
        {
        }

        bool RegisterAllObjects()
        {
            bool success = true;
            for (auto && [objectId, objectJson] : this->DataSource->GetRegisteredObjects())
            {
                success = RegisterObject(objectJson) && success;
            }
            return success;
        }

        bool RegisterObject(const json& objectJson)
        {
            auto idIter = objectJson.find("id");
            if (idIter == objectJson.end())
            {
                this->LogError("", "", "Object is missing required 'id' property.");
                return false;
            }

            PHXString objectId = idIter->get<PHXString>();

            auto baseIter = objectJson.find("base");
            if (baseIter == objectJson.end())
            {
                this->LogError("", "", "Object is missing required 'base' property.");
                return false;
            }

            PHXString baseId = baseIter->get<PHXString>();

            this->Catalog->EmplaceObjectRecord(objectId, "/base"_n, LDSTypedValue(baseId));

            json flat = objectJson.flatten();

            for (auto && [propertyPath, valueJson] : flat.items())
            {
                if (propertyPath == "/base" || propertyPath == "/id")
                {
                    continue;
                }

                LDSTypedValue value;
                if (!this->GetPropertyValueFromJson(valueJson, objectId, propertyPath, value))
                {
                    this->LogError(objectId, propertyPath, "Failed to read property value.");
                    return false;
                }

                this->Catalog->EmplaceObjectRecord(objectId, propertyPath, value);
            }

            return true;
        }
    };
}