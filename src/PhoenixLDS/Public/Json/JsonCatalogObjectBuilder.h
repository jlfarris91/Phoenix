
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
                this->LogError("Object is missing required 'id' property.");
                return false;
            }

            PHXString rootObjectId = idIter->get<PHXString>();

            if (this->Catalog->HasObject(rootObjectId))
            {
                this->LogError("Object with id '{}' has already been registered.", rootObjectId).Context(rootObjectId);
                return false;
            }

            auto baseIter = objectJson.find("base");
            if (baseIter == objectJson.end())
            {
                this->LogError("Object is missing required 'base' property.").Context(rootObjectId);
                return false;
            }

            PHXString baseId = baseIter->get<PHXString>();

            this->Catalog->EmplaceObjectRecord(rootObjectId, "/base"_n, LDSTypedValue(baseId));

            const LDSRecord* objectTypeRecord = this->Catalog->FindTypeRecordForObject(rootObjectId, "/type"_n);
            if (!objectTypeRecord)
            {
                this->LogError("No base object or type registered with id '{}'.", baseId).Context(rootObjectId);
                return false;
            }

            if (objectTypeRecord->GetValueType() != ELDSValueType::Object)
            {
                this->LogError("Root objects must be of type Object.").Context(rootObjectId);
                return false;
            }

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
                this->LogError("Could not find type of object.").Context(rootObjectId, jsonPath);
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
                this->LogError("Expected object reference to be a string value.").Context(rootObjectId, jsonPath);
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
                this->LogError("Expected array property.").Context(rootObjectId, jsonPath);
                return false;
            }

            PHXString itemTypePath = typePath + "/items";
            const LDSRecord* itemsTypeRecord = this->Catalog->FindTypeRecordForObject(rootObjectId, itemTypePath + "/type");
            if (!itemsTypeRecord)
            {
                this->LogError("Failed to find items type record.").Context(rootObjectId, jsonPath);
                return false;
            }

            TOptional<uint32> maxItems;
            if (const LDSRecord* maxItemsRecord = this->Catalog->FindTypeRecordForObject(rootObjectId, typePath + "/max_items"))
            {
                maxItems = maxItemsRecord->GetValueAs<uint32>();
            }

            uint32 itemCount = 0;
            for (auto && [index, item] : json.items())
            {
                PHXString itemObjectPath = std::format("{}/{}", jsonPath, index);
                if (!ProcessObject(rootObjectId, item, itemObjectPath, itemTypePath))
                {
                    return false;
                }
                ++itemCount;
                if (maxItems.IsSet() && itemCount == maxItems.Get())
                {
                    this->LogWarning("Object defines an array with {} items but the max allowed is {}", json.size(), maxItems.Get()).Context(rootObjectId, jsonPath);
                    break;
                }
            }

            this->Catalog->EmplaceObjectRecord(rootObjectId, jsonPath + "/size", LDSTypedValue(itemCount));

            return true;
        }

        bool ProcessPODProperty(const PHXString& rootObjectId, const json& json, const PHXString& path)
        {
            LDSTypedValue value;
            if (!this->GetPropertyValueFromJson(json, rootObjectId, path, value))
            {
                this->LogError("Failed to read property value.").Context(rootObjectId, path);
                return false;
            }

            this->Catalog->EmplaceObjectRecord(rootObjectId, path, value);
            return true;
        }
    };
}