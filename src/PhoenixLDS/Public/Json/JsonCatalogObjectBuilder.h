
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

            PHXString rootObjectId = idIter->get<PHXString>();

            if (this->Catalog->HasObject(rootObjectId))
            {
                this->LogError("", "", "Object with id '{}' has already been registered.", rootObjectId);
                return false;
            }

            auto baseIter = objectJson.find("base");
            if (baseIter == objectJson.end())
            {
                this->LogError("", "", "Object is missing required 'base' property.");
                return false;
            }

            PHXString baseId = baseIter->get<PHXString>();

            if (!this->Catalog->HasObject(baseId) && !this->Catalog->HasType(baseId))
            {
                this->LogError("", "", "No base object or type registered with id '{}'.", baseId);
                return false;
            }

            this->Catalog->EmplaceObjectRecord(rootObjectId, "/base"_n, LDSTypedValue(baseId));

            return ProcessObject(rootObjectId, objectJson, "", "");
        }

        bool ProcessObject(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath)
        {
            const LDSRecord* objectTypeRecord = this->Catalog->FindTypeRecordForObject(rootObjectId, typePath + "/type");
            if (objectTypeRecord == nullptr)
            {
                this->LogError(rootObjectId, jsonPath, "Could not find type of object.");
                return false;
            }

            ELDSValueType type = objectTypeRecord->GetValueType();

            if (type == ELDSValueType::Object)
            {
                return ProcessObjectProperties(rootObjectId, json, jsonPath, typePath);
            }

            if (type == ELDSValueType::ObjectRef)
            {
                return ProcessObjectRef(rootObjectId, json, jsonPath);
            }

            if (type == ELDSValueType::Array)
            {
                return ProcessArray(rootObjectId, json, jsonPath, typePath);
            }
            
            return ProcessPODProperty(rootObjectId, json, jsonPath);
        }

        bool ProcessObjectProperties(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath)
        {
            for (auto && [propName, propValue] : json.items())
            {
                if (propName == "base" || propName == "id")
                {
                    continue;
                }

                PHXString propertyJsonPath = jsonPath + "/" + propName;
                PHXString propertyTypePath = typePath + "/" + propName;
                if (!ProcessObject(rootObjectId, propValue, propertyJsonPath, propertyTypePath))
                {
                    return false;
                }
            }
            return true;
        }

        bool ProcessObjectRef(const PHXString& rootObjectId, const json& json, const PHXString& jsonPath)
        {
            if (!json.is_string())
            {
                this->LogError(rootObjectId, jsonPath, "Expected object reference to be a string value.");
                return false;
            }

            const PHXString& valueStr = json.get<PHXString>();
            FName valueId = FName(valueStr.data(), valueStr.length());

            LDSTypedValue value;
            value.Type = ELDSValueType::ObjectRef;
            value.Value.Name = valueId;

            this->Catalog->EmplaceObjectRecord(rootObjectId, jsonPath, value);
            return true;
        }

        bool ProcessArray(
            const PHXString& rootObjectId,
            const json& json,
            const PHXString& jsonPath,
            const PHXString& typePath)
        {
            if (!json.is_array())
            {
                this->LogError(rootObjectId, jsonPath, "Expected array property.");
                return false;
            }

            PHXString itemTypePath = typePath + "/items";
            const LDSRecord* itemsTypeRecord = this->Catalog->FindTypeRecordForObject(rootObjectId, itemTypePath + "/type");
            if (!itemsTypeRecord)
            {
                this->LogError(rootObjectId, jsonPath, "Failed to find items type record.");
                return false;
            }

            this->Catalog->EmplaceObjectRecord(rootObjectId, jsonPath + "/size", LDSTypedValue((uint32)json.size()));

            for (auto && [index, item] : json.items())
            {
                PHXString itemObjectPath = std::format("{}/{}", jsonPath, index);
                if (!ProcessObject(rootObjectId, item, itemObjectPath, itemTypePath))
                {
                    return false;
                }
            }

            return true;
        }

        bool ProcessPODProperty(const PHXString& rootObjectId, const json& json, const PHXString& path)
        {
            LDSTypedValue value;
            if (!this->GetPropertyValueFromJson(json, rootObjectId, path, value))
            {
                this->LogError(rootObjectId, path, "Failed to read property value.");
                return false;
            }

            this->Catalog->EmplaceObjectRecord(rootObjectId, path, value);
            return true;
        }
    };
}