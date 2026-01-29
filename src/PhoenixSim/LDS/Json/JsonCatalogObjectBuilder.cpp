#include "JsonCatalogObjectBuilder.h"

#include "JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

JsonCatalogObjectBuilder::JsonCatalogObjectBuilder(const JsonDataSource* dataSource, HeapLDSCatalog* catalog)
    : JsonCatalogBuilderBase(dataSource, catalog)
{
}

bool JsonCatalogObjectBuilder::RegisterAllObjects()
{
    bool success = true;
    for (const auto& objectJson : DataSource->GetRegisteredObjects() | std::views::values)
    {
        success = RegisterObject(objectJson) && success;
    }
    return success;
}

bool JsonCatalogObjectBuilder::RegisterObject(const json& objectJson)
{
    auto idIter = objectJson.find("id");
    if (idIter == objectJson.end())
    {
        LogError("Object is missing required 'id' property.");
        return false;
    }

    PHXString rootObjectId = idIter->get<PHXString>();

    if (Catalog->HasObject(rootObjectId))
    {
        LogInfo("Object with id '{}' has already been registered.", rootObjectId).Context(rootObjectId);
        return true;
    }

    auto baseIter = objectJson.find("base");
    if (baseIter == objectJson.end())
    {
        LogError("Object is missing required 'base' property.").Context(rootObjectId);
        return false;
    }

    PHXString baseId = baseIter->get<PHXString>();
    EmplaceObjectRecord(rootObjectId, "/base", LDSTypedValue(baseId));

    const LDSRecord* objectTypeRecord = FindTypeRecordForObject(rootObjectId, "/type"_n);
    if (!objectTypeRecord && DataSource)
    {
        const nlohmann::json* baseObjectJson = DataSource->FindObject(baseId);
        if (baseObjectJson && !Catalog->HasObject(baseId))
        {
            RegisterObject(*baseObjectJson);
            objectTypeRecord = FindTypeRecordForObject(rootObjectId, "/type"_n);
        }
    }

    if (!objectTypeRecord)
    {
        LogError("No base object or type registered with id '{}'.", baseId).Context(rootObjectId);
        return false;
    }

    if (objectTypeRecord->GetValueType() != ELDSValueType::Object)
    {
        LogError("Root objects must be of type Object.").Context(rootObjectId);
        return false;
    }

    return ProcessJsonObject(rootObjectId, objectJson, "", "");
}

bool JsonCatalogObjectBuilder::ProcessJsonObject(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& jsonPath,
    const PHXString& typePath)
{
    const LDSRecord* objectTypeRecord = FindTypeRecordForObject(rootObjectId, typePath + "/type");
    if (objectTypeRecord == nullptr)
    {
        LogError("Could not find type of object.").Context(rootObjectId, jsonPath);
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

bool JsonCatalogObjectBuilder::ProcessObjectProperties(
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

bool JsonCatalogObjectBuilder::ProcessObjectRef(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& jsonPath)
{
    if (!json.is_string())
    {
        LogError("Expected object reference to be a string value.").Context(rootObjectId, jsonPath);
        return false;
    }

    const PHXString& valueStr = json.get<PHXString>();

    LDSTypedValue value;
    value.Type = ELDSValueType::ObjectRef;
    value.Value.Name = valueStr;

    EmplaceObjectRecord(rootObjectId, jsonPath, value);
    return true;
}

bool JsonCatalogObjectBuilder::ProcessArray(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& jsonPath,
    const PHXString& typePath)
{
    if (!json.is_array())
    {
        LogError("Expected array property.").Context(rootObjectId, jsonPath);
        return false;
    }

    PHXString itemTypePath = typePath + "/items";
    const LDSRecord* itemsTypeRecord = FindTypeRecordForObject(rootObjectId, itemTypePath + "/type");
    if (!itemsTypeRecord)
    {
        LogError("Failed to find items type record.").Context(rootObjectId, jsonPath);
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
            LogWarning("Object defines an array with {} items but the max allowed is {}", json.size(), maxItems.Get()).Context(rootObjectId, jsonPath);
            break;
        }
    }

    EmplaceObjectRecord(rootObjectId, jsonPath + "/size", LDSTypedValue(itemCount));

    return true;
}

bool JsonCatalogObjectBuilder::ProcessEnum(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& jsonPath,
    const PHXString& typePath)
{
    if (!json.is_string())
    {
        LogError("Expected enum property to be a string value.").Context(rootObjectId, jsonPath);
        return false;
    }

    const PHXString& valueStr = json.get<PHXString>();

    FName typeId = Catalog->GetBaseTypeId(rootObjectId);
    if (FName::IsNoneOrEmpty(typeId))
    {
        LogError("Failed to find base type for object '{}'", rootObjectId).Context(rootObjectId, jsonPath);
        return false;
    }

    LDSEnumTypePtr enumType(LDSRecordPath(typeId, typePath));

    LDSEnumTypeItemPtr enumItem;
    if (!enumType.TryGetEnumItem(TypeQueryContext, valueStr, enumItem))
    {
        LogError("Could not find enum item named '{}'", valueStr).Context(rootObjectId, jsonPath);
        return false;
    }

    LDSTypedValue enumItemValue = enumItem.Value.GetRecordValue(TypeQueryContext);

    EmplaceObjectRecord(rootObjectId, jsonPath, enumItemValue);
    return true;
}

bool JsonCatalogObjectBuilder::ProcessEnumFlags(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& jsonPath,
    const PHXString& typePath)
{
    if (!json.is_string())
    {
        LogError("Expected enum flags property to be a '|'-delimited string value.").Context(rootObjectId, jsonPath);
        return false;
    }

    FName typeId = Catalog->GetBaseTypeId(rootObjectId);
    if (FName::IsNoneOrEmpty(typeId))
    {
        LogError("Failed to find base type for object '{}'", rootObjectId).Context(rootObjectId, jsonPath);
        return false;
    }

    LDSEnumTypePtr enumType(LDSRecordPath(typeId, typePath));

    ELDSValueType underlyingType = enumType.UnderlyingType.GetValue(TypeQueryContext);
    int32 flagsValue = 0;

    const PHXString& valueStr = json.get<PHXString>();
    PHXString token;
    std::istringstream tokenStream(valueStr);
    while (std::getline(tokenStream, token, '|'))
    {
        LDSEnumTypeItemPtr enumItem;
        if (!enumType.TryGetEnumItem(TypeQueryContext, token, enumItem))
        {
            LogError("Could not find enum item named '{}'", token).Context(rootObjectId, jsonPath);
            return false;
        }

        int32 flagValue = enumItem.Value.GetValue<int32>(TypeQueryContext);
        flagsValue |= flagValue;
    }

    EmplaceObjectRecord(rootObjectId, jsonPath, LDSTypedValue({ .Int32 = flagsValue}, underlyingType));
    return true;
}

bool JsonCatalogObjectBuilder::ProcessValueProperty(
    const PHXString& rootObjectId,
    const json& json,
    const PHXString& path)
{
    LDSTypedValue value;
    if (!GetPropertyValueFromJson(json, rootObjectId, path, value))
    {
        LogError("Failed to read property value.").Context(rootObjectId, path);
        return false;
    }

    EmplaceObjectRecord(rootObjectId, path, value);
    return true;
}

const LDSRecord* JsonCatalogObjectBuilder::FindTypeRecordForObject(
    const FName& objectId,
    const FName& propertyId)
{
    if (TypeIdOverride.IsSet())
    {
        return Catalog->FindTypeRecord(TypeIdOverride.Get(), propertyId);
    }

    return Catalog->FindTypeRecordForObject(objectId, propertyId);
}