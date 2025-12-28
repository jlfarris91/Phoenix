
#pragma once

#include <nlohmann/json.hpp>

#include "PhoenixSim/Name.h"
#include "PhoenixSim/Logging.h"
#include "PhoenixSim/LDS/LDSCatalog.h"
#include "PhoenixSim/LDS/LDSCatalogQueryContext.h"

namespace Phoenix::LDS::Json
{
    class JsonDataSource;

    struct PHOENIX_SIM_API JsonCatalogBuilderLogMessage : LogMessage
    {
        PHXString Id;
        PHXString PropertyPath;

        JsonCatalogBuilderLogMessage& Context(const PHXString& id, const PHXString& path = {})
        {
            Id = id;
            PropertyPath = path;
            return *this;
        }
    };

    template <class TCatalog = Catalog>
    struct JsonCatalogBuilderBase : ILogCollectionOwner<JsonCatalogBuilderLogMessage>
    {
        using json = nlohmann::json;

        JsonCatalogBuilderBase(const JsonDataSource* dataSource, TCatalog* catalog)
            : Catalog(catalog)
            , DataSource(dataSource)
            , ObjectQueryContext(catalog, ELDSCatalogRecordStore::Object)
            , TypeQueryContext(catalog, ELDSCatalogRecordStore::Type)
        {
            PHX_ASSERT(catalog);
        }

    protected:

        static bool GetValueFromJson(
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
                    outValue.Name = json.get<PHXString>();
                    return true;
                }
                if (json.is_number_integer())
                {
                    outValue.Name = json.get<hash32_t>();
                    return true;
                }
                break;
            case ELDSValueType::Value:
                if (json.is_number())
                {
                    outValue.Value = json.get<double>();
                    return true;
                }
                break;
            case ELDSValueType::Distance:
                if (json.is_number())
                {
                    outValue.Distance = json.get<double>();
                    return true;
                }
                break;
            case ELDSValueType::Degrees:
                if (json.is_number())
                {
                    outValue.Degrees = json.get<double>();
                    return true;
                }
                break;
            case ELDSValueType::Speed:
                if (json.is_number())
                {
                    outValue.Speed = json.get<double>();
                    return true;
                }
                break;
            case ELDSValueType::Time:
                if (json.is_number())
                {
                    outValue.Time = json.get<double>();
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
            case ELDSValueType::Text:
            case ELDSValueType::Asset:
            case ELDSValueType::Enum:
            case ELDSValueType::EnumFlags:
                if (json.is_string())
                {
                    outValue.Name = json.get<PHXString>();
                    return true;
                }
                break;
            case ELDSValueType::Unknown:
            case ELDSValueType::Array:
            case ELDSValueType::Object:
            case ELDSValueType::ObjectRef:
            case ELDSValueType::Expression:
                PHX_ASSERT(0); // Unexpected type
                break;
            }

            return false;
        }

        bool GetPropertyValueObjectRefFromJson(const json& json, LDSValue& value)
        {
            if (!json.is_string())
            {
                this->LogError("Expected object reference to be a string value.");
                return false;
            }

            const PHXString& valueStr = json.get<PHXString>();
            FName valueId = FName(valueStr.data(), valueStr.length());
            value.Name = valueId;

            return true;
        }

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const PHXString& propertyPath,
            uint32 pointerFirst,
            uint32 pointerLast,
            LDSTypedValue& outValue)
        {
            PHXString propertyPart = propertyPath.substr( pointerFirst, pointerLast - pointerFirst);
            FName propertyId = propertyPart;

            FName propertyTypeId = propertyId + "/type";
            const LDSRecord* metaRecord = Catalog->FindTypeRecordForObject(objectId, propertyTypeId);
            if (!metaRecord)
            {
                return false;
            }

            ELDSValueType type = metaRecord->GetValueType();

            if (type == ELDSValueType::Object)
            {
                // Just skip the name of the object
                pointerLast = static_cast<uint32>(propertyPath.find_first_of('/', pointerLast + 1));
                return GetPropertyValueFromJson(json, objectId, propertyPath, pointerFirst, pointerLast, outValue);
            }

            if (type == ELDSValueType::ObjectRef)
            {
                outValue.Type = ELDSValueType::ObjectRef;
                return GetPropertyValueObjectRefFromJson(json, outValue.Value);
            }

            if (type == ELDSValueType::Array)
            {
                PHXString nextPropertyPath;
                PHXString firstPart = propertyPath.substr(0, pointerLast);
                pointerLast = static_cast<uint32>(propertyPath.find_first_of('/', pointerLast + 1));
                if (pointerLast != Index<uint32>::None)
                {
                    PHXString lastPart = propertyPath.substr(pointerLast);
                    nextPropertyPath = std::format("{}/items{}", firstPart, lastPart);
                }
                else
                {
                    nextPropertyPath = firstPart + "/items";
                }
                pointerLast = static_cast<uint32>(nextPropertyPath.find_first_of('/', firstPart.length() + 6));
                return GetPropertyValueFromJson(json, objectId, nextPropertyPath, 0, pointerLast, outValue);
            }

            outValue.Type = metaRecord->GetValueAs<ELDSValueType>();
            return GetValueFromJson(json, outValue.Type, outValue.Value);
        }

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const PHXString& propertyPath,
            LDSTypedValue& outValue)
        {
            uint32 end = static_cast<uint32>(propertyPath.find_first_of('/', 1));
            if (end == Index<uint32>::None)
            {
                end = static_cast<uint32>(propertyPath.length());
            }
            return GetPropertyValueFromJson(json, objectId, propertyPath, 0, end, outValue);
        }

        TCatalog* Catalog;
        const JsonDataSource* DataSource;
        LDSCatalogQueryContext<TCatalog> ObjectQueryContext;
        LDSCatalogQueryContext<TCatalog> TypeQueryContext;
    };
}
