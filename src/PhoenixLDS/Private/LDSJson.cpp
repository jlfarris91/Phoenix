
#include "LDSJson.h"
#include "Platform.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

using json = nlohmann::json;

bool Json::FromJson(
    const json& json,
    ELDSValueType type,
    LDSValue& outValue)
{
    switch (type)
    {
    case ELDSValueType::Int32:
        if (json.is_number_integer())
        {
            outValue.Int32 = json.get<int32>();
            return true;
        }
        if (json.is_string())
        {
            const PHXString& str = json.get<PHXString>();
            outValue.Int32 = static_cast<int32>(strtoul(str.c_str(), nullptr, 16));
            return true;
        }
        break;
    case ELDSValueType::UInt32:
        if (json.is_number_integer())
        {
            outValue.UInt32 = json.get<uint32>();
            return true;
        }
        if (json.is_string())
        {
            const PHXString& str = json.get<PHXString>();
            outValue.UInt32 = static_cast<uint32>(strtoul(str.c_str(), nullptr, 16));
            return true;
        }
        break;
    case ELDSValueType::Name:
        if (json.is_string())
        {
            const auto& str = json.get<PHXString>();
            outValue.Name = Hashing::FNV1A32(str.data(), str.length());
            return true;
        }
        if (json.is_number_integer())
        {
            outValue.Name = json.get<hash32_t>();
            return true;
        }
        break;
    case ELDSValueType::Value:
        if (json.is_number_float())
        {
            outValue.Value = json.get<double>();
            return true;
        }
        break;
    case ELDSValueType::Distance:
        if (json.is_number_float())
        {
            outValue.Distance = json.get<double>();
            return true;
        }
        break;
    case ELDSValueType::Degrees:
        if (json.is_number_float())
        {
            outValue.Degrees = json.get<double>();
            return true;
        }
        break;
    case ELDSValueType::Speed:
        if (json.is_number_float())
        {
            outValue.Speed = json.get<double>();
            return true;
        }
        break;
    case ELDSValueType::Bool:
        if (json.is_boolean())
        {
            outValue.Bool = json.get<bool>();
            return true;
        }
        break;
    case ELDSValueType::Object:
        break;
    }

    return false;
}

bool Json::FromJson(
    const TFixedLDS<64>& lds,
    const TFixedLDS<64>& metadata,
    const json& json,
    const FName& archetypeId,
    const char* pointerStr,
    uint32 pointerFirst,
    uint32 pointerLast,
    LDSTypedValue& outValue)
{
    const char* subPointer = pointerStr + pointerFirst;
    uint32 subPointerLen = pointerLast - pointerFirst;

    FName propertyId = FName(subPointer, subPointerLen);

    // Find the type record for this archetype
    const LDSRecord* baseRecord = lds.FindRecord(archetypeId, "/base"_n);
    if (!baseRecord)
    {
        return false;
    }

    FName baseId = baseRecord->GetValueAs<FName>();
    while (!metadata.HasObject(baseId))
    {
        baseRecord = lds.FindRecord(baseId, "/base"_n);
        if (!baseRecord)
        {
            // Base archetype is not registered with the LDS
            return false;
        }
        baseId = baseRecord->GetValueAs<FName>();
    }

    FName propertyTypeId = propertyId + "/type";
    const LDSRecord* metaRecord = metadata.FindRecord(baseId, propertyTypeId);
    if (!metaRecord)
    {
        return false;
    }

    outValue.Type = metaRecord->GetValueAs<ELDSValueType>();
    return FromJson(json, outValue.Type, outValue.Value);
}

bool Json::FromJson(
    const TFixedLDS<64>& lds,
    const TFixedLDS<64>& metadata,
    const json& json,
    const FName& archetypeId,
    const json::json_pointer& pointer,
    LDSTypedValue& outValue)
{
    PHXString pointerStr = pointer.to_string();
    uint32 end = static_cast<uint32>(pointerStr.find_first_of('/', 1));
    if (end == Index<uint32>::None)
        end = static_cast<uint32>(pointerStr.length());
    return FromJson(lds, metadata, json, archetypeId, pointerStr.data(), 0, end, outValue);
}

bool Json::ProcessTypeRoot(TFixedLDS<64>& metadata, const json& typeJson)
{
    auto idIter = typeJson.find("id");
    if (idIter == typeJson.end())
    {
        // Error bad data
        return false;
    }

    PHXString typeIdStr = idIter->get<PHXString>();
    FName typeId = FName(typeIdStr.data(), typeIdStr.length());

    if (!ProcessObject(metadata, typeId, typeId, "", typeJson, ""))
    {
        return false;
    }

    // Only add the id record if we successfully processed the type
    metadata.EmplaceRecord_GetRef(typeId, "/id"_n, LDSTypedValue({ { .Name = typeId }, ELDSValueType::Name }));
    return true;
}

bool Json::ProcessObject(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const FName& objectId,
    const json& jsonObject,
    const PHXString& propertyPath)
{
    auto propsIter = jsonObject.find("properties");
    if (propsIter == jsonObject.end())
    {
        // Error: object type must have at least one property, right?
        return false;
    }

    const json& props = *propsIter;
    for (auto && [propName, propValue] : props.items())
    {
        if (!ProcessObject(metadata, rootObjectId, objectId, propName, propValue, propertyPath))
        {
            return false;
        }
    }

    return true;
}

bool Json::ProcessObjectRef(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const FName& objectId,
    const json& json,
    const PHXString& typeStr,
    const PHXString& propertyPath)
{
    auto indexOfHash = typeStr.find('#');
    if (indexOfHash == Index<size_t>::None)
    {
        // Error: expected hash character
        return false;
    }

    PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());
    FName refTypeId = FName(refTypeStr.data(), refTypeStr.length()); 

    // Record the type
    {
        PHXString typePropertyPath = propertyPath + "/type";
        FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
        LDSTypedValue typeValue = { { .Name = refTypeId }, ELDSValueType::ObjectRef };
        metadata.EmplaceRecord_GetRef(objectId, typePropertyPathId, typeValue);
    }

    // TODO (jfarris): what if we could also allow for inline property overrides on this referenced object?
    // I.E. Lancer references MeleeWeapon object but changes the damage to 100?
    //  1. We could automatically define a new archetype that inherits from MeleeWeapon and overrides damage to 100
    //     and swap out the reference automatically?

    return true;
}

bool Json::ProcessInlineObjectRef(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const FName& objectId,
    const json& jsonObject,
    const PHXString& typeStr,
    const PHXString& propertyPath)
{
    auto indexOfHash = typeStr.find('#');
    if (indexOfHash == Index<size_t>::None)
    {
        // Error: expected hash character
        return false;
    }

    PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());
    FName refTypeId = FName(refTypeStr.data(), refTypeStr.length());

    const LDSRecord* refTypeRecord = metadata.FindRecord(refTypeId, "/id"_n);
    if (!refTypeRecord)
    {
        // Error: could not find metadata for referenced type
        return false;
    }

    // Record the type
    {
        PHXString typePropertyPath = propertyPath + "/type";
        FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
        LDSTypedValue typeValue = LDSTypedValue({ .Name = refTypeId }, ELDSValueType::Object);
        metadata.EmplaceRecord_GetRef(rootObjectId, typePropertyPathId, typeValue);
    }

    // Copy records from the referenced type inline
    // TODO (jfarris): we need to create a new absolute property path given the relative path of the referenced object
    // To do this we will need the object's original json or utilize some intermediate format
    // metadata.ForEachRecord(
    //     refId,
    //     [&metadata, &rootObjectId, propertyPath](const LDSRecord& record)
    //     {
    //         FName fieldId = record.GetPropertyId();
    //         if (fieldId == "/type"_n)
    //         {
    //             return;
    //         }
    //
    //         metadata.EmplaceRecord_GetRef(rootObjectId, fieldId, record.GetValue());
    //     });

    // Process any default values
    // TODO (jfarris): record default values

    return true;
}

bool Json::ProcessObject(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const FName& objectId,
    const PHXString& key,
    const json& jsonObject,
    const PHXString& propertyPath)
{
    auto typeIter = jsonObject.find("type");
    if (typeIter == jsonObject.end())
    {
        // Error: expected type property
        return false;
    }

    PHXString typeStr = typeIter->get<PHXString>();

    PHXString newPropertyPath = propertyPath;
    if (!key.empty())
    {
        newPropertyPath += "/" + key;
    }

    // Defining a new object inline
    if (typeStr == "Object")
    {
        // Record the type
        PHXString typePropertyPath = newPropertyPath + "/type";
        FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
        metadata.EmplaceRecord_GetRef(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)ELDSValueType::Object }, ELDSValueType::UInt32));

        return ProcessObject(metadata, rootObjectId, objectId, jsonObject, newPropertyPath);
    }

    // A reference to another object
    // This must come before the inline object check since it is a superset
    if (typeStr.starts_with("ObjectRef"))
    {
        return ProcessObjectRef(metadata, rootObjectId, objectId, jsonObject, typeStr, newPropertyPath);
    }

    // Inline object using another defined type
    if (typeStr.starts_with("Object"))
    {
        return ProcessInlineObjectRef(metadata, rootObjectId, objectId, jsonObject, typeStr, newPropertyPath);
    }

    if (typeStr == "Array")
    {
        // Record the type
        PHXString typePropertyPath = newPropertyPath + "/type";
        FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
        metadata.EmplaceRecord_GetRef(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)ELDSValueType::Array }, ELDSValueType::UInt32));

        return ProcessArray(metadata, rootObjectId, objectId, jsonObject, newPropertyPath);
    }

    // Plain old data value types
    ELDSValueType valueType;
    if (TryParse(typeStr, valueType))
    {
        // Record the type
        PHXString typePropertyPath = newPropertyPath + "/type";
        FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
        metadata.EmplaceRecord_GetRef(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)valueType }, ELDSValueType::UInt32));

        return ProcessPODProperty(metadata, rootObjectId, key, jsonObject, valueType, newPropertyPath);
    }

    return false;
}

bool Json::ProcessArray(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const FName& objectId,
    const json& jsonObject,
    const PHXString& propertyPath)
{
    return true;
}

bool Json::ProcessPODProperty(
    TFixedLDS<64>& metadata,
    const FName& rootObjectId,
    const PHXString& propName,
    const json& propValue,
    ELDSValueType valueType,
    const PHXString& propertyPath)
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
            break;
        case ELDSValueType::Array:
        case ELDSValueType::Object:
        case ELDSValueType::ObjectRef:
            // Error: unexpected type
            PHX_ASSERT(0);
    }

    for (auto && [metaName, metaValue] : propValue.items())
    {
        PHXString fieldPtr = propertyPath + "/" + metaName;
        FName fieldId = FName(fieldPtr.data(), fieldPtr.length());

        if (metaName == "min")
        {
            int32 minVal = metaValue.get<int32>();
            LDSTypedValue value = { LDSValue(minVal), ELDSValueType::Int32 };
            metadata.EmplaceRecord_GetRef(rootObjectId, fieldId, value);
        }

        if (metaName == "max")
        {
            LDSTypedValue value;
            value.Type = ELDSValueType::Int32;
            value.Value.Int32 = metaValue.get<int32>();
            metadata.EmplaceRecord_GetRef(rootObjectId, fieldId, value);
        }

        if (metaName == "default")
        {
            LDSTypedValue value = { {}, valueType };
            if (!FromJson(metaValue, valueType, value.Value))
            {
                // Error: default value is not of expected type
                return false;
            }

            metadata.EmplaceRecord_GetRef(rootObjectId, fieldId, value);
        }
    }

    return true;
}

void Json::Test()
{
    TFixedLDS<64> lds;
    TFixedLDS<64> metadata;

    json colorJson = R"(
    {
        "id": "Color",
        "type": "UInt32",
        "format": "((?:[A-F]|[0-9])(?:[A-F]|[0-9])){3,4}",
        "min_len": 6,
        "max_len": 8,
        "default": "FFFFFFFF"
    }
    )"_json;

    json weaponJson = R"(
    {
        "id": "Weapon",
        "type": "Object",
        "properties": {
            "damage": {
                "type": "Int32",
                "default": 10,
                "min": 1,
                "max": 100
            },
            "range": {
                "type": "Distance",
                "default": 5.0
            }
        }
    }
    )"_json;

    json unitJson = R"(
    {
        "id": "Unit",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32",
                "default": 256,
                "min": 128,
                "max": 512
            },
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "#Color"
                },
                "max_items": 10
            },
            "testObjInline": {
                "type": "Object",
                "properties": {
                    "testSubInt": {
                        "type": "Int32",
                        "default": 100
                    }
                }
            },
            "testObjRef": {
                "type": "Object#Color",
                "default": "FFAABB"
            },
            "weaponInline": {
                "type": "Object#Weapon",
                "default": {
                    "damage": 50
                }
            },
            "weaponRef": {
                "type": "ObjectRef#Weapon",
                "default": null
            }
        }
    }
    )"_json;

    TArray<json> types;
    types.emplace_back(colorJson);
    types.emplace_back(unitJson);
    types.emplace_back(weaponJson);

    for (const auto& typeJson : types)
    {
        ProcessTypeRoot(metadata, typeJson);
    }

    json baseUnitJson = R"(
    {
        "id": "BaseUnit",
        "base": "Unit",
        "testInt": 456
    }
    )"_json;

    json lancerJson = R"(
    {
        "id": "Lancer",
        "base": "BaseUnit",
        "testInt": 123
    }
    )"_json;

    json baseWeaponJson = R"(
    {
        "id": "BaseWeapon",
        "base": "Weapon",
        "damage": 100
    }
    )"_json;

    json lancerWeaponJson = R"(
    {
        "id": "LancerWeapon",
        "base": "BaseWeapon",
        "damage": 75
    }
    )"_json;

    TArray<json> archetypes;
    archetypes.emplace_back(baseUnitJson);
    archetypes.emplace_back(baseWeaponJson);
    archetypes.emplace_back(lancerJson);
    archetypes.emplace_back(lancerWeaponJson);

    for (const auto& archetypeJson : archetypes)
    {
        auto idIter = archetypeJson.find("id");
        if (idIter == archetypeJson.end())
        {
            continue;
        }

        PHXString archetypeIdStr = idIter->get<PHXString>();
        FName archetypeId = FName(archetypeIdStr.data(), archetypeIdStr.length());

        auto baseIter = archetypeJson.find("base");
        if (baseIter == archetypeJson.end())
        {
            continue;
        }

        PHXString baseStr = baseIter->get<PHXString>();
        FName baseId = FName(baseStr.data(), baseStr.length());

        LDSTypedValue idValue = LDSTypedValue(LDSValue { .Name = archetypeId }, ELDSValueType::Name);
        lds.EmplaceRecord_GetRef(archetypeId, "/id"_n, idValue);

        LDSTypedValue baseValue = LDSTypedValue(LDSValue { .Name = baseId }, ELDSValueType::Name);
        lds.EmplaceRecord_GetRef(archetypeId, "/base"_n, baseValue);

        json flat = archetypeJson.flatten();

        for (auto && [name, valueJson] : flat.items())
        {
            if (name == "/base" || name == "/id")
            {
                continue;
            }

            FName fieldId = FName(name.data(), name.length());

            LDSTypedValue value;
            if (!FromJson(lds, metadata, valueJson, archetypeId, json::json_pointer(name), value))
            {
                continue;
            }

            lds.EmplaceRecord_GetRef(archetypeId, fieldId, value);
        }
    }

    LDSRecord* record = lds.FindRecord("Lancer"_n, "/testInt"_n);
    PHX_ASSERT(record);
    PHX_ASSERT(record->GetValue().Type == ELDSValueType::Int32);
    PHX_ASSERT(record->GetValue().Value.Int32 == 123);
    PHX_ASSERT(record->GetValueAs<int32>() == 123);
}
