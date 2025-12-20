#pragma once

#include "PhoenixSim/LDS/Json/JsonCatalogObjectBuilder.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
    JsonCatalogObjectBuilder<TCatalog>::JsonCatalogObjectBuilder(
        const JsonDataSource* dataSource,
        TCatalog* catalog)
        : JsonCatalogBuilderBase<TCatalog>(dataSource, catalog)
    {
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::RegisterAllObjects()
    {
        bool success = true;
        for (auto && [objectId, objectJson] : this->DataSource->GetRegisteredObjects())
        {
            success = RegisterObject(objectJson) && success;
        }
        return success;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::RegisterObject(const json& objectJson)
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

        EmplaceObjectRecord(rootObjectId, "/base", LDSTypedValue(baseId));

        const LDSRecord* objectTypeRecord = FindTypeRecordForObject(rootObjectId, "/type"_n);
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

        return ProcessJsonObject(rootObjectId, objectJson, "", "");
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessJsonObject(
        const PHXString& rootObjectId,
        const json& json,
        const PHXString& jsonPath,
        const PHXString& typePath)
    {
        const LDSRecord* objectTypeRecord = FindTypeRecordForObject(rootObjectId, typePath + "/type");
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

        if (type == ELDSValueType::Enum)
        {
            return ProcessEnum(rootObjectId, json, jsonPath, typePath);
        }

        if (type == ELDSValueType::EnumFlags)
        {
            return ProcessEnumFlags(rootObjectId, json, jsonPath, typePath);
        }

        return ProcessValueProperty(rootObjectId, json, jsonPath);
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessObjectProperties(
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
            if (!ProcessJsonObject(rootObjectId, propValue, propertyJsonPath, propertyTypePath))
            {
                return false;
            }
        }
        return true;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessObjectRef(
        const PHXString& rootObjectId,
        const json& json,
        const PHXString& jsonPath)
    {
        if (!json.is_string())
        {
            this->LogError("Expected object reference to be a string value.").Context(rootObjectId, jsonPath);
            return false;
        }

        const PHXString& valueStr = json.get<PHXString>();

        LDSTypedValue value;
        value.Type = ELDSValueType::ObjectRef;
        value.Value.Name = valueStr;

        EmplaceObjectRecord(rootObjectId, jsonPath, value);
        return true;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessArray(
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
        const LDSRecord* itemsTypeRecord = FindTypeRecordForObject(rootObjectId, itemTypePath + "/type");
        if (!itemsTypeRecord)
        {
            this->LogError("Failed to find items type record.").Context(rootObjectId, jsonPath);
            return false;
        }

        TOptional<uint32> maxItems;
        if (const LDSRecord* maxItemsRecord = FindTypeRecordForObject(rootObjectId, typePath + "/max_items"))
        {
            maxItems = maxItemsRecord->GetValueAs<uint32>();
        }

        uint32 itemCount = 0;
        for (auto && [index, item] : json.items())
        {
            PHXString itemObjectPath = std::format("{}/{}", jsonPath, index);
            if (!ProcessJsonObject(rootObjectId, item, itemObjectPath, itemTypePath))
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

        EmplaceObjectRecord(rootObjectId, jsonPath + "/size", LDSTypedValue(itemCount));

        return true;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessEnum(
        const PHXString& rootObjectId,
        const json& json,
        const PHXString& jsonPath,
        const PHXString& typePath)
    {
        if (!json.is_string())
        {
            this->LogError("Expected enum property to be a string value.").Context(rootObjectId, jsonPath);
            return false;
        }

        const PHXString& valueStr = json.get<PHXString>();

        FName typeId = this->Catalog->GetBaseTypeId(rootObjectId);
        if (FName::IsNoneOrEmpty(typeId))
        {
            this->LogError("Failed to find base type for object '{}'", rootObjectId).Context(rootObjectId, jsonPath);
            return false;
        }

        LDSEnumTypePtr enumType(LDSRecordPath(typeId, typePath));

        LDSEnumTypeItemPtr enumItem;
        if (!enumType.TryGetEnumItem(this->TypeQueryContext, valueStr, enumItem))
        {
            this->LogError("Could not find enum item named '{}'", valueStr).Context(rootObjectId, jsonPath);
            return false;
        }

        LDSTypedValue enumItemValue = enumItem.Value.GetRecordValue(this->TypeQueryContext);

        EmplaceObjectRecord(rootObjectId, jsonPath, enumItemValue);
        return true;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessEnumFlags(
        const PHXString& rootObjectId,
        const json& json,
        const PHXString& jsonPath,
        const PHXString& typePath)
    {
        if (!json.is_string())
        {
            this->LogError("Expected enum flags property to be a '|'-delimited string value.").Context(rootObjectId, jsonPath);
            return false;
        }

        FName typeId = this->Catalog->GetBaseTypeId(rootObjectId);
        if (FName::IsNoneOrEmpty(typeId))
        {
            this->LogError("Failed to find base type for object '{}'", rootObjectId).Context(rootObjectId, jsonPath);
            return false;
        }

        LDSEnumTypePtr enumType(LDSRecordPath(typeId, typePath));

        ELDSValueType underlyingType = enumType.UnderlyingType.GetValue(this->TypeQueryContext);
        int32 flagsValue = 0;

        const PHXString& valueStr = json.get<PHXString>();
        PHXString token;
        std::istringstream tokenStream(valueStr);
        while (std::getline(tokenStream, token, '|'))
        {
            LDSEnumTypeItemPtr enumItem;
            if (!enumType.TryGetEnumItem(this->TypeQueryContext, token, enumItem))
            {
                this->LogError("Could not find enum item named '{}'", token).Context(rootObjectId, jsonPath);
                return false;
            }

            int32 flagValue = enumItem.Value.GetValue<int32>(this->TypeQueryContext);
            flagsValue |= flagValue;
        }

        EmplaceObjectRecord(rootObjectId, jsonPath, LDSTypedValue({ .Int32 = flagsValue}, underlyingType));
        return true;
    }

    template <class TCatalog>
    bool JsonCatalogObjectBuilder<TCatalog>::ProcessValueProperty(
        const PHXString& rootObjectId,
        const json& json,
        const PHXString& path)
    {
        LDSTypedValue value;
        if (!this->GetPropertyValueFromJson(json, rootObjectId, path, value))
        {
            this->LogError("Failed to read property value.").Context(rootObjectId, path);
            return false;
        }

        EmplaceObjectRecord(rootObjectId, path, value);
        return true;
    }

    template <class TCatalog>
    const LDSRecord* JsonCatalogObjectBuilder<TCatalog>::FindTypeRecordForObject(
        const FName& objectId,
        const FName& propertyId)
    {
        if (TypeIdOverride.IsSet())
        {
            return this->Catalog->FindTypeRecord(TypeIdOverride.Get(), propertyId);
        }

        return this->Catalog->FindTypeRecordForObject(objectId, propertyId);
    }

    template <class TCatalog>
    template <class ... TArgs>
    void JsonCatalogObjectBuilder<TCatalog>::EmplaceObjectRecord(
        const PHXString& rootObjectId,
        const PHXString& path,
        TArgs&&... args)
    {
        PHXString finalPath = path;
        if (PathPostFix.IsSet())
        {
            finalPath += PathPostFix.Get();
        }

        if (RecordStore == ELDSCatalogRecordStore::Type)
        {
            this->Catalog->EmplaceTypeRecord(rootObjectId, finalPath, std::forward<TArgs>(args)...);
        }
        else
        {
            this->Catalog->EmplaceObjectRecord(rootObjectId, finalPath, std::forward<TArgs>(args)...);
        }
    }
}
