#include "JsonCatalogTypeBuilder.h"

#include "JsonCatalogObjectBuilder.h"
#include "JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

JsonCatalogTypeBuilder::JsonCatalogTypeBuilder(const JsonDataSource* dataSource, HeapLDSCatalog* catalog)
    : JsonCatalogBuilderBase(dataSource, catalog)
{
}

bool JsonCatalogTypeBuilder::RegisterAllTypes()
{
    bool success = true;
    for (auto && [typeId, typeJson] : DataSource->GetRegisteredTypes())
    {
        success = RegisterType(typeJson) && success;
    }
    return success;
}

bool JsonCatalogTypeBuilder::RegisterType(const json& typeJson)
{
    auto idIter = typeJson.find("id");
    if (idIter == typeJson.end())
    {
        LogError("Type is missing required 'id' property.");
        return false;
    }

    std::string typeId = idIter->get<std::string>();

    if (Catalog->HasType(typeId))
    {
        LogInfo("Type with id '{}' has already been registered.", typeId).Context(typeId);
        return true;
    }

    auto baseIter = typeJson.find("base");
    if (baseIter != typeJson.end())
    {
        std::string baseId = baseIter->get<std::string>();

        const LDSRecord* baseTypeRecord = Catalog->FindTypeRecord(baseId, "/type"_n);
        if (!baseTypeRecord && DataSource)
        {
            const nlohmann::json* baseTypeJson = DataSource->FindType(baseId);
            if (baseTypeJson && !Catalog->HasType(baseId))
            {
                RegisterType(*baseTypeJson);
                baseTypeRecord = Catalog->FindTypeRecord(baseId, "/type"_n);
            }
        }

        if (!baseTypeRecord)
        {
            LogError("No base type registered with id '{}'.", baseId).Context(typeId);
            return false;
        }

        Catalog->EmplaceTypeRecord(typeId, "/base"_n, LDSTypedValue(baseId));
    }

    if (DataSource)
    {
        const auto& implements = DataSource->GetInterfacesOfType(typeId);
        if (!implements.empty())
        {
            Catalog->EmplaceTypeRecord(typeId, "/implements/size"_n, LDSTypedValue((uint32)implements.size()));
            uint32 index = 0;
            for (const std::string& implementId : implements)
            {
                char buffer[16];
                size_t len = snprintf(buffer, 16, "/implements/%u", index++);
                Catalog->EmplaceTypeRecord(typeId, FName(buffer, len), LDSTypedValue(implementId));
            }
        }
    }

    if (!ProcessJsonObject(typeId, typeJson, ""))
    {
        return false;
    }

    // Only add the id record if we successfully processed the type
    Catalog->EmplaceTypeRecord(typeId, "/id"_n, LDSTypedValue(typeId));
    return true;
}

bool JsonCatalogTypeBuilder::ProcessJsonObject(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    auto typeIter = jsonObject.find("type");
    if (typeIter == jsonObject.end())
    {
        LogError("Type is missing required 'type' property.").Context(rootTypeId, propertyPath);
        return false;
    }

    std::string typeStr = typeIter->get<std::string>();

    // Defining a new object inline
    if (typeStr == "Object")
    {
        return ProcessObject(rootTypeId, jsonObject, propertyPath);
    }

    // A reference to another object
    // This must come before the inline object check since it is a superset
    if (typeStr.starts_with("ObjectRef"))
    {
        return ProcessObjectRef(rootTypeId, typeStr, propertyPath);
    }

    // Embedded object using another defined type
    if (typeStr.starts_with("Object"))
    {
        return ProcessEmbeddedObject(rootTypeId, jsonObject, typeStr, propertyPath);
    }

    if (typeStr.starts_with("Asset"))
    {
        // TODO (jfarris): implement assets?
        return ProcessValueProperty(rootTypeId, jsonObject, ELDSValueType::Asset, propertyPath);
    }

    if (typeStr.starts_with("Expression"))
    {
        // TODO (jfarris): implement expressions
        return true;
    }

    if (typeStr == "Enum")
    {
        return ProcessEnumProperty(rootTypeId, jsonObject, propertyPath);
    }

    if (typeStr == "EnumFlags")
    {
        return ProcessEnumFlagsProperty(rootTypeId, jsonObject, propertyPath);
    }

    if (typeStr == "Array")
    {
        return ProcessArray(rootTypeId, jsonObject, propertyPath);
    }

    // Plain old data value types
    ELDSValueType valueType;
    if (TryParse(typeStr, valueType))
    {
        // Record the type
        std::string typePropertyPath = propertyPath + "/type";
        Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, LDSTypedValue(valueType));

        return ProcessValueProperty(rootTypeId, jsonObject, valueType, propertyPath);
    }

    LogError("Unknown object type {}", typeStr);
    return false;
}

bool JsonCatalogTypeBuilder::ProcessObject(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    // Record the type
    std::string typePropertyPath = propertyPath + "/type";
    Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, LDSTypedValue(ELDSValueType::Object));

    if (!ProcessObjectProperties(rootTypeId, jsonObject, propertyPath))
    {
        return false;
    }
        
    auto defaultsIter = jsonObject.find("default");
    if (defaultsIter != jsonObject.end() &&
        !ProcessObjectPropertyDefaults(rootTypeId, *defaultsIter, propertyPath))
    {
        return false;
    }

    return true;
}

bool JsonCatalogTypeBuilder::ProcessObjectProperties(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    auto propsIter = jsonObject.find("properties");
    if (propsIter == jsonObject.end())
    {
        LogWarning("Object type is missing expected 'properties' property.").Context(rootTypeId, propertyPath);
        return true;
    }

    const json& props = *propsIter;
    if (props.empty())
    {
        LogWarning("Object type 'properties' is empty.").Context(rootTypeId, propertyPath);
        return true;
    }

    for (auto && [propName, propValue] : props.items())
    {
        if (!ProcessJsonObject(rootTypeId, propValue, propertyPath + "/" + propName))
        {
            return false;
        }
    }

    return true;
}

bool JsonCatalogTypeBuilder::ProcessObjectPropertyDefaults(
    const std::string& rootTypeId,
    const json& defaultsObject,
    const std::string& propertyPath)
{
    if (!defaultsObject.is_object())
    {
        LogError("Expected 'default' property to be an object.").Context(rootTypeId, propertyPath);
        return false;
    }

    JsonCatalogObjectBuilder objectBuilder(DataSource, Catalog);
    objectBuilder.TypeIdOverride = rootTypeId;
    objectBuilder.PathPostFix = "/default";
    objectBuilder.RecordStore = ELDSCatalogRecordStore::Type;
    return objectBuilder.ProcessObjectProperties(rootTypeId, defaultsObject, propertyPath, propertyPath);
}

bool JsonCatalogTypeBuilder::ProcessObjectRef(
    const std::string& rootTypeId,
    const std::string& typeStr,
    const std::string& propertyPath)
{
    if (!DataSource)
    {
        LogError("A data source is required for object references.").Context(rootTypeId, propertyPath);
        return false;
    }

    auto indexOfHash = typeStr.find('#');
    if (indexOfHash == Index<size_t>::None)
    {
        LogError("Malformed object reference. Expected 'Object#' followed by a type id.").Context(rootTypeId, propertyPath);
        return false;
    }

    std::string refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

    if (!DataSource->HasTypeOrInterface(refTypeStr))
    {
        LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootTypeId, propertyPath);
        return false;
    }

    // Record the type
    {
        std::string typePropertyPath = propertyPath + "/type";
        LDSTypedValue typeValue = { { .Name = refTypeStr }, ELDSValueType::ObjectRef };
        Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, typeValue);
    }

    // TODO (jfarris): what if we could also allow for inline property overrides on this referenced object?
    // I.E. Lancer references MeleeWeapon object but changes the damage to 100?
    //  1. We could automatically define a new object that inherits from MeleeWeapon and overrides damage to 100
    //     and swap out the reference automatically?

    return true;
}

bool JsonCatalogTypeBuilder::ProcessEmbeddedObject(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& typeStr,
    const std::string& propertyPath)
{
    if (!DataSource)
    {
        LogError("A data source is required for embedded objects.").Context(rootTypeId, propertyPath);
        return false;
    }

    auto indexOfHash = typeStr.find('#');
    if (indexOfHash == Index<size_t>::None)
    {
        LogError("Malformed embedded object reference. Expected 'Object#' followed by a type id.").Context(rootTypeId, propertyPath);
        return false;
    }

    std::string refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

    const nlohmann::json* typeJson = DataSource->FindType(refTypeStr);
    if (!typeJson)
    {
        LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootTypeId, propertyPath);
        return false;
    }

    if (!ProcessJsonObject(rootTypeId, *typeJson, propertyPath))
    {
        return false;
    }

    return true;
}

bool JsonCatalogTypeBuilder::ProcessArray(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    auto itemsIter = jsonObject.find("items");
    if (itemsIter == jsonObject.end())
    {
        LogError("Array is missing required 'items' property.").Context(rootTypeId, propertyPath);
        return false;
    }

    const json& items = *itemsIter;

    if (!ProcessJsonObject(rootTypeId, items, propertyPath + "/items"))
    {
        return false;
    }

    // Record the type
    Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::Array));

    auto minItemsIter = jsonObject.find("min_items");
    if (minItemsIter != jsonObject.end())
    {
        uint32 minItems = std::max(minItemsIter->get<int32>(), 0);
        Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/min_items", LDSTypedValue(minItems));
    }

    auto maxItemsIter = jsonObject.find("max_items");
    if (maxItemsIter != jsonObject.end())
    {
        uint32 maxItems = std::max(maxItemsIter->get<int32>(), 0);
        Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/max_items", LDSTypedValue(maxItems));
    }

    // TODO (jfarris): allow user to specify default item values here

    return true;
}

bool JsonCatalogTypeBuilder::ProcessValueProperty(
    const std::string& rootTypeId,
    const json& propValue,
    ELDSValueType valueType,
    const std::string& propertyPath)
{
    switch (valueType)
    {
        case ELDSValueType::Int32:
        case ELDSValueType::UInt32:
        case ELDSValueType::Value:
        case ELDSValueType::Distance:
        case ELDSValueType::Degrees:
        case ELDSValueType::Speed:
        case ELDSValueType::Name:
        case ELDSValueType::Bool:
        case ELDSValueType::Text:
        case ELDSValueType::Asset:
            break;
        case ELDSValueType::Array:
        case ELDSValueType::Object:
        case ELDSValueType::ObjectRef:
        case ELDSValueType::Unknown:
            // Error: unexpected type
            PHX_ASSERT(0);
            break;
    }

    for (auto && [metaName, metaValue] : propValue.items())
    {
        std::string fieldId = propertyPath + "/" + metaName;

        if (metaName == "min")
        {
            int32 minVal = metaValue.get<int32>();
            LDSTypedValue value = { LDSValue(minVal), ELDSValueType::Int32 };
            Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
        }

        if (metaName == "max")
        {
            LDSTypedValue value;
            value.Type = ELDSValueType::Int32;
            value.Value.Int32 = metaValue.get<int32>();
            Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
        }

        if (metaName == "default")
        {
            LDSTypedValue value = { {}, valueType };
            if (!GetValueFromJson(metaValue, valueType, value.Value))
            {
                LogError("Unexpected default value type.").Context(rootTypeId, propertyPath);
                return false;
            }

            Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
        }
    }

    // Record the type
    Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(valueType));

    return true;
}

bool JsonCatalogTypeBuilder::IsValidEnumUnderlyingType(ELDSValueType valueType)
{
    switch (valueType)
    {
        case ELDSValueType::Bool:
        case ELDSValueType::Int32:
        case ELDSValueType::UInt32:
        case ELDSValueType::Name:
        case ELDSValueType::Value:
        case ELDSValueType::Distance:
        case ELDSValueType::Degrees:
        case ELDSValueType::Speed:
        case ELDSValueType::Time:
        case ELDSValueType::Text:
        case ELDSValueType::Asset:
        case ELDSValueType::ObjectRef:
            return true;
        default:
            return false;
    }
}

bool JsonCatalogTypeBuilder::ProcessEnumProperty(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    auto itemsIter = jsonObject.find("items");
    if (itemsIter == jsonObject.end())
    {
        LogError("Enum is missing required 'items' property.").Context(rootTypeId, propertyPath);
        return false;
    }

    std::string itemsPropertyPath = propertyPath + "/items";
    const json& items = *itemsIter;

    if (!items.is_array())
    {
        LogError("Expected enum items property to be an array of strings.").Context(rootTypeId, itemsPropertyPath);
        return false;
    }

    if (items.empty())
    {
        LogError("Expected at least one enum item.").Context(rootTypeId, itemsPropertyPath);
        return false;
    }

    // Record the property type
    Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::Enum));

    // Record the underlying enum value type
    TOptional<ELDSValueType> customUnderlyingValueType;
    {
        ELDSValueType underlyingValueType = ELDSValueType::UInt32;

        auto underlyingValueTypeIter = jsonObject.find("underlying_type");
        if (underlyingValueTypeIter != jsonObject.end())
        {
            std::string underlyingValueTypeStr = underlyingValueTypeIter->get<std::string>();
            if (!TryParse(underlyingValueTypeStr, underlyingValueType))
            {
                LogError("Failed to parse underlying type.").Context(rootTypeId, propertyPath + "/underlying_type");
                return false;
            }
            if (!IsValidEnumUnderlyingType(underlyingValueType))
            {
                LogError("Unsupported underlying type '{}'.", underlyingValueTypeStr).Context(rootTypeId, propertyPath + "/underlying_type");
                return false;
            }
            customUnderlyingValueType = underlyingValueType;
        }

        Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/underlying_type", LDSTypedValue(underlyingValueType));
    }

    // Record the number of enum items
    Catalog->EmplaceTypeRecord(rootTypeId, itemsPropertyPath + "/size", LDSTypedValue((uint32)items.size()));

    bool success = true;

    // Record each enum item value
    for (auto && [key, item] : items.items())
    {
        std::string itemPropertyPath = itemsPropertyPath + '/' + key;
        uint32 index = atoi(key.c_str());
        if (!ProcessEnumPropertyItem(rootTypeId, item, itemPropertyPath, index, customUnderlyingValueType))
        {
            success = false;
        }
    }

    return success;
}

bool JsonCatalogTypeBuilder::ProcessEnumPropertyItem(
    const std::string& rootTypeId,
    const json& itemJson,
    const std::string& itemPath,
    uint32 itemIndex,
    TOptional<ELDSValueType> underlyingValueType)
{
    FName itemKey;
    LDSTypedValue itemValue;

    if (underlyingValueType.IsSet())
    {
        // Allow users to define enum values specifically when the underlying type isn't Name
        // ie { "Test": 5 }
        if (!itemJson.is_object() || itemJson.empty())
        {
            LogError("Expected object for enum item because underlying type was specified.").Context(rootTypeId, itemPath);
            return false;
        }

        auto itemKVPJson = itemJson.items().begin();
        itemKey = itemKVPJson.key();

        if (!GetValueFromJson(itemKVPJson.value(), underlyingValueType.Get(), itemValue.Value))
        {
            LogError("Failed to read enum item value.").Context(rootTypeId, itemPath);
            return false;
        }
    }
    else if (itemJson.is_string())
    {
        itemKey = itemJson.get<std::string>();
        itemValue = LDSTypedValue(itemIndex);
    }
    else
    {
        LogError("Expected type for enum item.").Context(rootTypeId, itemPath);
        return false;
    }

    Catalog->EmplaceTypeRecord(rootTypeId, itemPath + "/key", itemKey);
    Catalog->EmplaceTypeRecord(rootTypeId, itemPath + "/value", itemValue);
    return true;
}

bool JsonCatalogTypeBuilder::ProcessEnumFlagsProperty(
    const std::string& rootTypeId,
    const json& jsonObject,
    const std::string& propertyPath)
{
    auto itemsIter = jsonObject.find("items");
    if (itemsIter == jsonObject.end())
    {
        LogError("Enum is missing required 'items' property.").Context(rootTypeId, propertyPath);
        return false;
    }

    std::string itemsPropertyPath = propertyPath + "/items";
    const json& items = *itemsIter;

    if (!items.is_array())
    {
        LogError("Expected enum items property to be an array of strings.").Context(rootTypeId, itemsPropertyPath);
        return false;
    }

    if (items.empty())
    {
        LogError("Expected at least one enum item.").Context(rootTypeId, itemsPropertyPath);
        return false;
    }

    // Record the property type
    Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::EnumFlags));

    // Record the underlying enum value type
    TOptional<ELDSValueType> customUnderlyingValueType;
    {
        ELDSValueType underlyingValueType = ELDSValueType::UInt32;

        auto underlyingValueTypeIter = jsonObject.find("underlying_type");
        if (underlyingValueTypeIter != jsonObject.end())
        {
            std::string underlyingValueTypeStr = underlyingValueTypeIter->get<std::string>();
            if (!TryParse(underlyingValueTypeStr, underlyingValueType))
            {
                LogError("Failed to parse underlying type.").Context(rootTypeId, propertyPath + "/underlying_type");
                return false;
            }
            if (!IsValidEnumUnderlyingType(underlyingValueType))
            {
                LogError("Unsupported underlying type '{}'.", underlyingValueTypeStr).Context(rootTypeId, propertyPath + "/underlying_type");
                return false;
            }
            customUnderlyingValueType = underlyingValueType;
        }

        Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/underlying_type", LDSTypedValue(underlyingValueType));
    }

    // Record the number of enum items
    Catalog->EmplaceTypeRecord(rootTypeId, itemsPropertyPath + "/size", LDSTypedValue((uint32)items.size()));

    bool success = true;

    // Record each enum item value
    for (auto && [key, item] : items.items())
    {
        std::string itemPropertyPath = itemsPropertyPath + '/' + key;
        uint32 index = atoi(key.c_str());
        if (!ProcessEnumPropertyItem(rootTypeId, item, itemPropertyPath, index, customUnderlyingValueType))
        {
            success = false;
        }
    }

    return success;
}