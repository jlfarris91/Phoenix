
#pragma once

#include "JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
    PHOENIX_LDS_API struct JsonCatalogObjectBuilder : JsonCatalogBuilderBase<TCatalog>
    {
        using json = nlohmann::json;

        JsonCatalogObjectBuilder(TCatalog* catalog)
            : JsonCatalogBuilderBase<TCatalog>(catalog)
        {
        }

        bool RegisterObject(const json& objectJson)
        {
            auto idIter = objectJson.find("id");
            if (idIter == objectJson.end())
            {
                return false;
            }

            PHXString objectIdStr = idIter->get<PHXString>();
            FName objectId = FName(objectIdStr.data(), objectIdStr.length());

            auto baseIter = objectJson.find("base");
            if (baseIter == objectJson.end())
            {
                return false;
            }

            PHXString baseStr = baseIter->get<PHXString>();
            FName baseId = FName(baseStr.data(), baseStr.length());

            this->CatalogPtr->EmplaceObjectRecord(objectId, "/id"_n, LDSTypedValue(objectId));
            this->CatalogPtr->EmplaceObjectRecord(objectId, "/base"_n, LDSTypedValue(baseId));

            json flat = objectJson.flatten();

            for (auto && [name, valueJson] : flat.items())
            {
                if (name == "/base" || name == "/id")
                {
                    continue;
                }

                FName fieldId = FName(name.data(), name.length());

                LDSTypedValue value;
                if (!this->GetPropertyValueFromJson(valueJson, objectId, json::json_pointer(name), value))
                {
                    return false;
                }

                this->CatalogPtr->EmplaceObjectRecord(objectId, fieldId, value);
            }

            return true;
        }
    };
}