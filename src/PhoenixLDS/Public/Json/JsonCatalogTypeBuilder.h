
#pragma once

#include "JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
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
            const PHXString& rootObjectId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto typeIter = jsonObject.find("type");
            if (typeIter == jsonObject.end())
            {
                this->LogError("Type is missing required 'type' property.").Context(rootObjectId, propertyPath);
                return false;
            }

            PHXString typeStr = typeIter->get<PHXString>();

            // Defining a new object inline
            if (typeStr == "Object")
            {
                // Record the type
                PHXString typePropertyPath = propertyPath + "/type";
                this->Catalog->EmplaceTypeRecord(rootObjectId, typePropertyPath, LDSTypedValue(ELDSValueType::Object));

                return ProcessObjectProperties(rootObjectId, jsonObject, propertyPath);
            }

            // A reference to another object
            // This must come before the inline object check since it is a superset
            if (typeStr.starts_with("ObjectRef"))
            {
                return ProcessObjectRef(rootObjectId, typeStr, propertyPath);
            }

            // Embedded object using another defined type
            if (typeStr.starts_with("Object"))
            {
                return ProcessEmbeddedObject(rootObjectId, typeStr, propertyPath);
            }

            if (typeStr.starts_with("Asset"))
            {
                // TODO (jfarris): implement assets?
                return ProcessPODProperty(rootObjectId, jsonObject, ELDSValueType::Asset, propertyPath);
            }

            if (typeStr.starts_with("Expression"))
            {
                // TODO (jfarris): implement expressions
                return true;
            }

            if (typeStr == "Enum")
            {
                // TODO (jfarris): implement enum
                return true;
            }

            if (typeStr == "Flags")
            {
                // TODO (jfarris): implement enum flags
                return true;
            }

            if (typeStr == "Array")
            {
                // Record the type
                PHXString typePropertyPath = propertyPath + "/type";
                this->Catalog->EmplaceTypeRecord(rootObjectId, typePropertyPath, LDSTypedValue(ELDSValueType::Array));

                return ProcessArray(rootObjectId, jsonObject, propertyPath);
            }

            // Plain old data value types
            ELDSValueType valueType;
            if (TryParse(typeStr, valueType))
            {
                // Record the type
                PHXString typePropertyPath = propertyPath + "/type";
                this->Catalog->EmplaceTypeRecord(rootObjectId, typePropertyPath, LDSTypedValue(valueType));

                return ProcessPODProperty(rootObjectId, jsonObject, valueType, propertyPath);
            }

            this->LogError("Unknown object type {}", typeStr);
            return false;
        }

        bool ProcessObjectProperties(
            const PHXString& rootObjectId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto propsIter = jsonObject.find("properties");
            if (propsIter == jsonObject.end())
            {
                this->LogWarning("Object type is missing expected 'properties' property.").Context(rootObjectId, propertyPath);
                return true;
            }

            const json& props = *propsIter;
            if (props.empty())
            {
                this->LogWarning("Object type 'properties' is empty.").Context(rootObjectId, propertyPath);
                return true;
            }

            for (auto && [propName, propValue] : props.items())
            {
                if (!ProcessObject(rootObjectId, propValue, propertyPath + "/" + propName))
                {
                    return false;
                }
            }

            return true;
        }

        bool ProcessObjectRef(
            const PHXString& rootObjectId,
            const PHXString& typeStr,
            const PHXString& propertyPath)
        {
            if (!this->DataSource)
            {
                this->LogError("A data source is required for object references.").Context(rootObjectId, propertyPath);
                return false;
            }

            auto indexOfHash = typeStr.find('#');
            if (indexOfHash == Index<size_t>::None)
            {
                this->LogError("Malformed object reference. Expected 'Object#' followed by a type id.").Context(rootObjectId, propertyPath);
                return false;
            }

            PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

            if (!this->DataSource->HasTypeOrInterface(refTypeStr))
            {
                this->LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootObjectId, propertyPath);
                return false;
            }

            // Record the type
            {
                PHXString typePropertyPath = propertyPath + "/type";
                LDSTypedValue typeValue = { { .Name = refTypeStr }, ELDSValueType::ObjectRef };
                this->Catalog->EmplaceTypeRecord(rootObjectId, typePropertyPath, typeValue);
            }

            // TODO (jfarris): what if we could also allow for inline property overrides on this referenced object?
            // I.E. Lancer references MeleeWeapon object but changes the damage to 100?
            //  1. We could automatically define a new object that inherits from MeleeWeapon and overrides damage to 100
            //     and swap out the reference automatically?

            return true;
        }

        bool ProcessEmbeddedObject(
            const PHXString& rootObjectId,
            const PHXString& typeStr,
            const PHXString& propertyPath)
        {
            if (!this->DataSource)
            {
                this->LogError("A data source is required for embedded objects.").Context(rootObjectId, propertyPath);
                return false;
            }

            auto indexOfHash = typeStr.find('#');
            if (indexOfHash == Index<size_t>::None)
            {
                this->LogError("Malformed embedded object reference. Expected 'Object#' followed by a type id.").Context(rootObjectId, propertyPath);
                return false;
            }

            PHXString refTypeStr = typeStr.substr(indexOfHash + 1, typeStr.length());

            const nlohmann::json* typeJson = this->DataSource->FindType(refTypeStr);
            if (!typeJson)
            {
                this->LogError("Could not find type with id '{}' in data source.", refTypeStr).Context(rootObjectId, propertyPath);
                return false;
            }

            if (!ProcessObject(rootObjectId, *typeJson, propertyPath))
            {
                return false;
            }

            // Process any default values
            // TODO (jfarris): record default values

            return true;
        }

        bool ProcessArray(
            const PHXString& rootObjectId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            auto itemsIter = jsonObject.find("items");
            if (itemsIter == jsonObject.end())
            {
                this->LogError("Array is missing required 'items' property.").Context(rootObjectId, propertyPath);
                return false;
            }

            const json& items = *itemsIter;

            if (!ProcessObject(rootObjectId, items, propertyPath + "/items"))
            {
                return false;
            }

            auto minItemsIter = jsonObject.find("min_items");
            if (minItemsIter != jsonObject.end())
            {
                uint32 minItems = std::max(minItemsIter->get<int32>(), 0);
                this->Catalog->EmplaceTypeRecord(rootObjectId, propertyPath + "/min_items", LDSTypedValue(minItems));
            }

            auto maxItemsIter = jsonObject.find("max_items");
            if (maxItemsIter != jsonObject.end())
            {
                uint32 maxItems = std::max(maxItemsIter->get<int32>(), 0);
                this->Catalog->EmplaceTypeRecord(rootObjectId, propertyPath + "/max_items", LDSTypedValue(maxItems));
            }

            // TODO (jfarris): allow user to specify default item values here

            return true;
        }

        bool ProcessPODProperty(
            const PHXString& rootObjectId,
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
                    this->Catalog->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }

                if (metaName == "max")
                {
                    LDSTypedValue value;
                    value.Type = ELDSValueType::Int32;
                    value.Value.Int32 = metaValue.get<int32>();
                    this->Catalog->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }

                if (metaName == "default")
                {
                    LDSTypedValue value = { {}, valueType };
                    if (!this->GetPropertyValueFromJson(metaValue, valueType, value.Value))
                    {
                        this->LogError("Unexpected default value type.").Context(rootObjectId, propertyPath);
                        return false;
                    }

                    this->Catalog->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }
            }

            return true;
        }
    };
}