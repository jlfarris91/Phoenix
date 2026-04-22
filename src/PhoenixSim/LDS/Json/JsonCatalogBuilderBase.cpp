#include "PhoenixSim/LDS/Json/JsonCatalogBuilderBase.h"

#include <charconv>
#include <string_view>
#include <system_error>

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

JsonCatalogBuilderBase::JsonCatalogBuilderBase(
    const JsonDataSource* dataSource,
    HeapLDSCatalog* catalog)
    : Catalog(catalog)
    , DataSource(dataSource)
    , ObjectQueryContext(catalog, ELDSCatalogRecordStore::Object)
    , TypeQueryContext(catalog, ELDSCatalogRecordStore::Type)
{
    PHX_ASSERT(catalog);
}

namespace
{
    template <class T>
    bool TryParseHexString(const std::string_view& str, T& outValue, std::string& outErrorMessage)
    {
        const char* first = str.data();
        const char* last = str.data() + str.size();

        if (str.starts_with('#'))
        {
            first += 1; // Skip the '#' character
        }

        T value = 0;
        auto [ptr, ec] = std::from_chars(first, last, value, 16);

        if (ec == std::errc{})
        {
            outValue = value;
            return true;
        }

        if (ec == std::errc::invalid_argument)
        {
            outErrorMessage = "not a number";
            return false;
        }

        if (ec == std::errc::result_out_of_range)
        {
            outErrorMessage = "out of range";
            return false;
        }

        return false;
    }
}

bool JsonCatalogBuilderBase::GetValueFromJson(
    const json& json,
    ELDSValueType type,
    LDSValue& outValue,
    std::string& outErrorMessage)
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
                const std::string& str = json.get<std::string>();
                std::string errorMessage;
                if (!TryParseHexString(str, outValue.Int32, errorMessage))
                {
                    outErrorMessage = std::format("Failed to parse hex value: {}", errorMessage);
                    return false;
                }
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
                std::string str = json.get<std::string>();
                std::string errorMessage;
                if (!TryParseHexString(str, outValue.UInt32, errorMessage))
                {
                    outErrorMessage = std::format("Failed to parse hex value: {}", errorMessage);
                    return false;
                }
                return true;
            }
            break;
        case ELDSValueType::Name:
            if (json.is_string())
            {
                outValue.Name = json.get<std::string>();
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
                outValue.Name = json.get<std::string>();
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

bool JsonCatalogBuilderBase::GetPropertyValueObjectRefFromJson(const json& json, LDSValue& value)
{
    if (!json.is_string())
    {
        this->LogError("Expected object reference to be a string value.");
        return false;
    }

    const std::string& valueStr = json.get<std::string>();
    FName valueId = FName(valueStr.data(), valueStr.length());
    value.Name = valueId;

    return true;
}

bool JsonCatalogBuilderBase::GetPropertyValueFromJson(
    const json& json,
    const std::string& objectId,
    const std::string& propertyPath,
    uint32 pointerFirst,
    uint32 pointerLast,
    LDSTypedValue& outValue)
{
    std::string propertyPart = propertyPath.substr( pointerFirst, pointerLast - pointerFirst);
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
        std::string nextPropertyPath;
        std::string firstPart = propertyPath.substr(0, pointerLast);
        pointerLast = static_cast<uint32>(propertyPath.find_first_of('/', pointerLast + 1));
        if (pointerLast != Index<uint32>::None)
        {
            std::string lastPart = propertyPath.substr(pointerLast);
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

    std::string errorMessage;
    if (!GetValueFromJson(json, outValue.Type, outValue.Value, errorMessage))
    {
        LogError("{}", errorMessage).Context(objectId, propertyPath);
        return false;
    }

    return true;
}

bool JsonCatalogBuilderBase::GetPropertyValueFromJson(
    const json& json,
    const std::string& objectId,
    const std::string& propertyPath,
    LDSTypedValue& outValue)
{
    uint32 end = static_cast<uint32>(propertyPath.find_first_of('/', 1));
    if (end == Index<uint32>::None)
    {
        end = static_cast<uint32>(propertyPath.length());
    }
    return GetPropertyValueFromJson(json, objectId, propertyPath, 0, end, outValue);
}
