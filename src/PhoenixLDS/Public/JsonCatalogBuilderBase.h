
#pragma once

#include "DLLExport.h"
#include "LDSCatalog.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog>
    PHOENIX_LDS_API struct JsonCatalogBuilderBase
    {
        using json = nlohmann::json;

        JsonCatalogBuilderBase(TCatalog* catalog)
            : CatalogPtr(catalog)
        {
            PHX_ASSERT(catalog);
        }

    protected:

        static bool GetPropertyValueFromJson(
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
            case ELDSValueType::ObjectRef:
            case ELDSValueType::Array:
            case ELDSValueType::Unknown:
                PHX_ASSERT(0); // Unexpected type
                break;
            }

            return false;
        }

        bool GetPropertyValueObjectRefFromJson(
            const json& json,
            const FName& expectedTypeId,
            LDSValue& value)
        {
            if (!json.is_string())
            {
                // Error: expected object ref to be a string value
                return false;
            }

            const PHXString& valueStr = json.get<PHXString>();
            FName valueId = FName(valueStr.data(), valueStr.length());
            value.Name = valueId;

            // TODO (jfarris): can't check the type unless it has already been added to the catalog....
            // return CatalogPtr->IsType(valueId, expectedTypeId);
            return true;
        }

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const char* pointerStr,
            uint32 pointerFirst,
            uint32 pointerLast,
            LDSTypedValue& outValue)
        {
            const char* subPointer = pointerStr + pointerFirst;
            uint32 subPointerLen = pointerLast - pointerFirst;

            FName propertyId = FName(subPointer, subPointerLen);

            FName typeId = CatalogPtr->GetBaseTypeId(objectId);
            if (typeId == FName::None)
            {
                // Error: type is not registered
                return false;
            }

            FName propertyTypeId = propertyId + "/type";
            const LDSRecord* metaRecord = CatalogPtr->FindTypeRecord(typeId, propertyTypeId);
            if (!metaRecord)
            {
                return false;
            }

            auto type = metaRecord->GetValueType();
            if (type == ELDSValueType::ObjectRef)
            {
                FName expectedType = metaRecord->GetValueAs<FName>();
                return GetPropertyValueObjectRefFromJson(json, expectedType, outValue.Value);
            }

            outValue.Type = metaRecord->GetValueAs<ELDSValueType>();
            return GetPropertyValueFromJson(json, outValue.Type, outValue.Value);
        }

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const json::json_pointer& pointer,
            LDSTypedValue& outValue)
        {
            PHXString pointerStr = pointer.to_string();
            uint32 end = static_cast<uint32>(pointerStr.find_first_of('/', 1));
            if (end == Index<uint32>::None)
                end = static_cast<uint32>(pointerStr.length());
            return GetPropertyValueFromJson(json, objectId, pointerStr.data(), 0, end, outValue);
        }

        TCatalog* CatalogPtr;
    };
}