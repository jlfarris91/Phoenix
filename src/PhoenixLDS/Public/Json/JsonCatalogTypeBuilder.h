
#pragma once

#include "JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog = Catalog>
    PHOENIX_LDS_API struct JsonCatalogTypeBuilder : JsonCatalogBuilderBase<TCatalog>
    {
        using json = nlohmann::json;

        JsonCatalogTypeBuilder(const JsonDataSource* dataSource, TCatalog* catalog)
            : JsonCatalogBuilderBase<TCatalog>(dataSource, catalog)
        {
        }

        bool RegisterAllTypes()
        {
            bool success = true;
            for (auto && [typeId, typeJson] : this->DataSource->GetRegisteredTypes())
            {
                success = RegisterType(typeJson) && success;
            }
            return success;
        }

        bool RegisterType(const json& typeJson)
        {
            auto idIter = typeJson.find("id");
            if (idIter == typeJson.end())
            {
                this->LogError("Type is missing required 'id' property.");
                return false;
            }

            PHXString typeId = idIter->get<PHXString>();

            if (this->Catalog->HasType(typeId))
            {
                this->LogError("Type with id '{}' has already been registered.", typeId).Context(typeId);
                return false;
            }

            if (this->DataSource)
            {
                const auto& implements = this->DataSource->GetInterfacesOfType(typeId);
                if (!implements.empty())
                {
                    this->Catalog->EmplaceTypeRecord(typeId, "/implements/size"_n, LDSTypedValue((uint32)implements.size()));
                    uint32 index = 0;
                    for (const PHXString& implementId : implements)
                    {
                        char buffer[16];
                        size_t len = snprintf(buffer, 16, "/implements/%u", index++);
                        this->Catalog->EmplaceTypeRecord(typeId, FName(buffer, len), LDSTypedValue(implementId));
                    }
                }
            }

            if (!ProcessObject(typeId, typeJson, ""))
            {
                return false;
            }

            // Only add the id record if we successfully processed the type
            this->Catalog->EmplaceTypeRecord(typeId, "/id"_n, LDSTypedValue(typeId));
            return true;
        }

    private:

        bool ProcessObject(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto typeIter = jsonObject.find("type");
            if (typeIter == jsonObject.end())
            {
                this->LogError("Type is missing required 'type' property.").Context(rootTypeId, propertyPath);
                return false;
            }

            PHXString typeStr = typeIter->get<PHXString>();

            // Defining a new object inline
            if (typeStr == "Object")
            {
                // Record the type
                PHXString typePropertyPath = propertyPath + "/type";
                this->Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, LDSTypedValue(ELDSValueType::Object));

                return ProcessObjectProperties(rootTypeId, jsonObject, propertyPath);
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
                return ProcessEmbeddedObject(rootTypeId, typeStr, propertyPath);
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
                PHXString typePropertyPath = propertyPath + "/type";
                this->Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, LDSTypedValue(valueType));

                return ProcessValueProperty(rootTypeId, jsonObject, valueType, propertyPath);
            }

            this->LogError("Unknown object type {}", typeStr);
            return false;
        }

        bool ProcessObjectProperties(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto propsIter = jsonObject.find("properties");
            if (propsIter == jsonObject.end())
            {
                this->LogWarning("Object type is missing expected 'properties' property.").Context(rootTypeId, propertyPath);
                return true;
            }

            const json& props = *propsIter;
            if (props.empty())
            {
                this->LogWarning("Object type 'properties' is empty.").Context(rootTypeId, propertyPath);
                return true;
            }

            for (auto && [propName, propValue] : props.items())
            {
                if (!ProcessObject(rootTypeId, propValue, propertyPath + "/" + propName))
                {
                    return false;
                }
            }

            return true;
        }

        bool ProcessObjectRef(
            const PHXString& rootTypeId,
            const PHXString& typeStr,
            const PHXString& propertyPath)
        {
            if (!this->DataSource)
            {
                this->LogError("A data source is required for object references.").Context(rootTypeId, propertyPath);
                return false;
            }

            auto indexOfHash = typeStr.find('#');
            if (indexOfHash == Index<size_t>::None)
            {
                this->LogError("Malformed object reference. Expected 'Object#' followed by a type id.").Context(rootTypeId, propertyPath);
                return false;
            }

            PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

            if (!this->DataSource->HasTypeOrInterface(refTypeStr))
            {
                this->LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootTypeId, propertyPath);
                return false;
            }

            // Record the type
            {
                PHXString typePropertyPath = propertyPath + "/type";
                LDSTypedValue typeValue = { { .Name = refTypeStr }, ELDSValueType::ObjectRef };
                this->Catalog->EmplaceTypeRecord(rootTypeId, typePropertyPath, typeValue);
            }

            // TODO (jfarris): what if we could also allow for inline property overrides on this referenced object?
            // I.E. Lancer references MeleeWeapon object but changes the damage to 100?
            //  1. We could automatically define a new object that inherits from MeleeWeapon and overrides damage to 100
            //     and swap out the reference automatically?

            return true;
        }

        bool ProcessEmbeddedObject(
            const PHXString& rootTypeId,
            const PHXString& typeStr,
            const PHXString& propertyPath)
        {
            if (!this->DataSource)
            {
                this->LogError("A data source is required for embedded objects.").Context(rootTypeId, propertyPath);
                return false;
            }

            auto indexOfHash = typeStr.find('#');
            if (indexOfHash == Index<size_t>::None)
            {
                this->LogError("Malformed embedded object reference. Expected 'Object#' followed by a type id.").Context(rootTypeId, propertyPath);
                return false;
            }

            PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

            const nlohmann::json* typeJson = this->DataSource->FindType(refTypeStr);
            if (!typeJson)
            {
                this->LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootTypeId, propertyPath);
                return false;
            }

            if (!ProcessObject(rootTypeId, *typeJson, propertyPath))
            {
                return false;
            }

            // Process any default values
            // TODO (jfarris): record default values

            return true;
        }

        bool ProcessArray(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto itemsIter = jsonObject.find("items");
            if (itemsIter == jsonObject.end())
            {
                this->LogError("Array is missing required 'items' property.").Context(rootTypeId, propertyPath);
                return false;
            }

            const json& items = *itemsIter;

            if (!ProcessObject(rootTypeId, items, propertyPath + "/items"))
            {
                return false;
            }

            // Record the type
            this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::Array));

            auto minItemsIter = jsonObject.find("min_items");
            if (minItemsIter != jsonObject.end())
            {
                uint32 minItems = std::max(minItemsIter->get<int32>(), 0);
                this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/min_items", LDSTypedValue(minItems));
            }

            auto maxItemsIter = jsonObject.find("max_items");
            if (maxItemsIter != jsonObject.end())
            {
                uint32 maxItems = std::max(maxItemsIter->get<int32>(), 0);
                this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/max_items", LDSTypedValue(maxItems));
            }

            // TODO (jfarris): allow user to specify default item values here

            return true;
        }

        bool ProcessValueProperty(
            const PHXString& rootTypeId,
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
                PHXString fieldId = propertyPath + "/" + metaName;

                if (metaName == "min")
                {
                    int32 minVal = metaValue.get<int32>();
                    LDSTypedValue value = { LDSValue(minVal), ELDSValueType::Int32 };
                    this->Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
                }

                if (metaName == "max")
                {
                    LDSTypedValue value;
                    value.Type = ELDSValueType::Int32;
                    value.Value.Int32 = metaValue.get<int32>();
                    this->Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
                }

                if (metaName == "default")
                {
                    LDSTypedValue value = { {}, valueType };
                    if (!this->GetValueFromJson(metaValue, valueType, value.Value))
                    {
                        this->LogError("Unexpected default value type.").Context(rootTypeId, propertyPath);
                        return false;
                    }

                    this->Catalog->EmplaceTypeRecord(rootTypeId, fieldId, value);
                }
            }

            // Record the type
            this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(valueType));

            return true;
        }

        bool IsValidEnumUnderlyingType(ELDSValueType valueType)
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

        bool ProcessEnumProperty(const PHXString& rootTypeId, const json& jsonObject, const PHXString& propertyPath)
        {
            auto itemsIter = jsonObject.find("items");
            if (itemsIter == jsonObject.end())
            {
                this->LogError("Enum is missing required 'items' property.").Context(rootTypeId, propertyPath);
                return false;
            }

            PHXString itemsPropertyPath = propertyPath + "/items";
            const json& items = *itemsIter;

            if (!items.is_array())
            {
                this->LogError("Expected enum items property to be an array of strings.").Context(rootTypeId, itemsPropertyPath);
                return false;
            }

            if (items.empty())
            {
                this->LogError("Expected at least one enum item.").Context(rootTypeId, itemsPropertyPath);
                return false;
            }

            // Record the property type
            this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::Enum));

            // Record the underlying enum value type
            TOptional<ELDSValueType> customUnderlyingValueType;
            {
                ELDSValueType underlyingValueType = ELDSValueType::UInt32;

                auto underlyingValueTypeIter = jsonObject.find("underlying_type");
                if (underlyingValueTypeIter != jsonObject.end())
                {
                    PHXString underlyingValueTypeStr = underlyingValueTypeIter->get<PHXString>();
                    if (!TryParse(underlyingValueTypeStr, underlyingValueType))
                    {
                        this->LogError("Failed to parse underlying type.").Context(rootTypeId, propertyPath + "/underlying_type");
                        return false;
                    }
                    if (!IsValidEnumUnderlyingType(underlyingValueType))
                    {
                        this->LogError("Unsupported underlying type '{}'.", underlyingValueTypeStr).Context(rootTypeId, propertyPath + "/underlying_type");
                        return false;
                    }
                    customUnderlyingValueType = underlyingValueType;
                }

                this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/underlying_type", LDSTypedValue(underlyingValueType));
            }

            // Record the number of enum items
            this->Catalog->EmplaceTypeRecord(rootTypeId, itemsPropertyPath + "/size", LDSTypedValue((uint32)items.size()));

            bool success = true;

            // Record each enum item value
            for (auto && [key, item] : items.items())
            {
                PHXString itemPropertyPath = itemsPropertyPath + '/' + key;
                uint32 index = atoi(key.c_str());
                if (!ProcessEnumPropertyItem(rootTypeId, item, itemPropertyPath, index, customUnderlyingValueType))
                {
                    success = false;
                }
            }

            return success;
        }

        bool ProcessEnumPropertyItem(
            const PHXString& rootTypeId,
            const json& itemJson,
            const PHXString& itemPath,
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
                    this->LogError("Expected object for enum item because underlying type was specified.").Context(rootTypeId, itemPath);
                    return false;
                }

                auto itemKVPJson = itemJson.items().begin();
                itemKey = itemKVPJson.key();

                if (!this->GetValueFromJson(itemKVPJson.value(), underlyingValueType.Get(), itemValue.Value))
                {
                    this->LogError("Failed to read enum item value.").Context(rootTypeId, itemPath);
                    return false;
                }
            }
            else if (itemJson.is_string())
            {
                itemKey = itemJson.get<PHXString>();
                itemValue = LDSTypedValue(itemIndex);
            }
            else
            {
                this->LogError("Expected type for enum item.").Context(rootTypeId, itemPath);
                return false;
            }

            this->Catalog->EmplaceTypeRecord(rootTypeId, itemPath + "/key", itemKey);
            this->Catalog->EmplaceTypeRecord(rootTypeId, itemPath + "/value", itemValue);
            return true;
        }

        bool ProcessEnumFlagsProperty(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto itemsIter = jsonObject.find("items");
            if (itemsIter == jsonObject.end())
            {
                this->LogError("Enum is missing required 'items' property.").Context(rootTypeId, propertyPath);
                return false;
            }

            PHXString itemsPropertyPath = propertyPath + "/items";
            const json& items = *itemsIter;

            if (!items.is_array())
            {
                this->LogError("Expected enum items property to be an array of strings.").Context(rootTypeId, itemsPropertyPath);
                return false;
            }

            if (items.empty())
            {
                this->LogError("Expected at least one enum item.").Context(rootTypeId, itemsPropertyPath);
                return false;
            }

            // Record the property type
            this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/type", LDSTypedValue(ELDSValueType::EnumFlags));

            // Record the underlying enum value type
            TOptional<ELDSValueType> customUnderlyingValueType;
            {
                ELDSValueType underlyingValueType = ELDSValueType::UInt32;

                auto underlyingValueTypeIter = jsonObject.find("underlying_type");
                if (underlyingValueTypeIter != jsonObject.end())
                {
                    PHXString underlyingValueTypeStr = underlyingValueTypeIter->get<PHXString>();
                    if (!TryParse(underlyingValueTypeStr, underlyingValueType))
                    {
                        this->LogError("Failed to parse underlying type.").Context(rootTypeId, propertyPath + "/underlying_type");
                        return false;
                    }
                    if (!IsValidEnumUnderlyingType(underlyingValueType))
                    {
                        this->LogError("Unsupported underlying type '{}'.", underlyingValueTypeStr).Context(rootTypeId, propertyPath + "/underlying_type");
                        return false;
                    }
                    customUnderlyingValueType = underlyingValueType;
                }

                this->Catalog->EmplaceTypeRecord(rootTypeId, propertyPath + "/underlying_type", LDSTypedValue(underlyingValueType));
            }

            // Record the number of enum items
            this->Catalog->EmplaceTypeRecord(rootTypeId, itemsPropertyPath + "/size", LDSTypedValue((uint32)items.size()));

            bool success = true;

            // Record each enum item value
            for (auto && [key, item] : items.items())
            {
                PHXString itemPropertyPath = itemsPropertyPath + '/' + key;
                uint32 index = atoi(key.c_str());
                if (!ProcessEnumPropertyItem(rootTypeId, item, itemPropertyPath, index, customUnderlyingValueType))
                {
                    success = false;
                }
            }

            return success;
        }
    };
}