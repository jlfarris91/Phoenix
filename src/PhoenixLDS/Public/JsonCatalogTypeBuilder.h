
#pragma once

#include "JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
    PHOENIX_LDS_API struct JsonCatalogTypeBuilder : JsonCatalogBuilderBase<TCatalog>
    {
        using json = nlohmann::json;

        JsonCatalogTypeBuilder(TCatalog* catalog)
            : JsonCatalogBuilderBase<TCatalog>(catalog)
        {
        }

        bool RegisterType(const json& typeJson)
        {
            auto idIter = typeJson.find("id");
            if (idIter == typeJson.end())
            {
                // Error bad data
                return false;
            }

            PHXString typeIdStr = idIter->get<PHXString>();
            FName typeId = FName(typeIdStr.data(), typeIdStr.length());

            if (!ProcessObject(typeId, typeId, "", typeJson, ""))
            {
                return false;
            }

            // Only add the id record if we successfully processed the type
            this->CatalogPtr->EmplaceTypeRecord(typeId, "/id"_n, LDSTypedValue({ { .Name = typeId }, ELDSValueType::Name }));
            return true;
        }

    private:

        bool ProcessObject(
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
                this->CatalogPtr->EmplaceTypeRecord(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)ELDSValueType::Object }, ELDSValueType::UInt32));

                return ProcessObjectProperties(rootObjectId, objectId, jsonObject, newPropertyPath);
            }

            // A reference to another object
            // This must come before the inline object check since it is a superset
            if (typeStr.starts_with("ObjectRef"))
            {
                return ProcessObjectRef(rootObjectId, objectId, jsonObject, typeStr, newPropertyPath);
            }

            // Inline object using another defined type
            if (typeStr.starts_with("Object"))
            {
                return ProcessInlineObjectRef(rootObjectId, objectId, jsonObject, typeStr, newPropertyPath);
            }

            if (typeStr == "Array")
            {
                // Record the type
                PHXString typePropertyPath = newPropertyPath + "/type";
                FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
                this->CatalogPtr->EmplaceTypeRecord(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)ELDSValueType::Array }, ELDSValueType::UInt32));

                return ProcessArray(rootObjectId, objectId, jsonObject, newPropertyPath);
            }

            // Plain old data value types
            ELDSValueType valueType;
            if (TryParse(typeStr, valueType))
            {
                // Record the type
                PHXString typePropertyPath = newPropertyPath + "/type";
                FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
                this->CatalogPtr->EmplaceTypeRecord(rootObjectId, typePropertyPathId, LDSTypedValue({ .UInt32 = (uint32)valueType }, ELDSValueType::UInt32));

                return ProcessPODProperty(rootObjectId, key, jsonObject, valueType, newPropertyPath);
            }

            return false;
        }

        bool ProcessObjectProperties(
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
            if (props.empty())
            {
                // Error: object type must have at least one property, right?
                return false;
            }
            
            for (auto && [propName, propValue] : props.items())
            {
                if (!ProcessObject(rootObjectId, objectId, propName, propValue, propertyPath))
                {
                    return false;
                }
            }

            return true;
        }

        bool ProcessObjectRef(
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

            if (!this->CatalogPtr->HasType(refTypeId))
            {
                // Error: referenced type does not exist
                return false;
            }

            // Record the type
            {
                PHXString typePropertyPath = propertyPath + "/type";
                FName typePropertyPathId = FName(typePropertyPath.data(), typePropertyPath.length());
                LDSTypedValue typeValue = { { .Name = refTypeId }, ELDSValueType::ObjectRef };
                this->CatalogPtr->EmplaceTypeRecord(objectId, typePropertyPathId, typeValue);
            }

            // TODO (jfarris): what if we could also allow for inline property overrides on this referenced object?
            // I.E. Lancer references MeleeWeapon object but changes the damage to 100?
            //  1. We could automatically define a new object that inherits from MeleeWeapon and overrides damage to 100
            //     and swap out the reference automatically?

            return true;
        }

        bool ProcessInlineObjectRef(
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

            const LDSRecord* refTypeRecord = this->CatalogPtr->FindTypeRecord(refTypeId, "/id"_n);
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
                this->CatalogPtr->EmplaceTypeRecord(rootObjectId, typePropertyPathId, typeValue);
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

        bool ProcessArray(
            const FName& rootObjectId,
            const FName& objectId,
            const json& jsonObject,
            const PHXString& propertyPath)
        {
            return true;
        }

        bool ProcessPODProperty(
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
                case ELDSValueType::Unknown:
                    // Error: unexpected type
                    PHX_ASSERT(0);
                    break;
            }

            for (auto && [metaName, metaValue] : propValue.items())
            {
                PHXString fieldPtr = propertyPath + "/" + metaName;
                FName fieldId = FName(fieldPtr.data(), fieldPtr.length());

                if (metaName == "min")
                {
                    int32 minVal = metaValue.get<int32>();
                    LDSTypedValue value = { LDSValue(minVal), ELDSValueType::Int32 };
                    this->CatalogPtr->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }

                if (metaName == "max")
                {
                    LDSTypedValue value;
                    value.Type = ELDSValueType::Int32;
                    value.Value.Int32 = metaValue.get<int32>();
                    this->CatalogPtr->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }

                if (metaName == "default")
                {
                    LDSTypedValue value = { {}, valueType };
                    if (!this->GetPropertyValueFromJson(metaValue, valueType, value.Value))
                    {
                        // Error: default value is not of expected type
                        return false;
                    }

                    this->CatalogPtr->EmplaceTypeRecord(rootObjectId, fieldId, value);
                }
            }

            return true;
        }
    };
}