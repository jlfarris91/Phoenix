#pragma once

#include "Phoenix.Sim/LDS/Json/JsonCatalogBuilderBase.h"
#include "Phoenix.Sim/Containers/Optional.h"

namespace Phoenix::LDS::Json
{
    struct PHOENIX_SIM_API JsonCatalogTypeBuilder : JsonCatalogBuilderBase
    {
        using json = nlohmann::json;

        JsonCatalogTypeBuilder(const JsonDataSource* dataSource, HeapLDSCatalog* catalog);

        bool RegisterAllTypes();

        bool RegisterType(const json& typeJson);

    private:

        bool ProcessJsonObject(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& propertyPath);

        bool ProcessObject(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& propertyPath);

        bool ProcessObjectProperties(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& propertyPath);

        bool ProcessObjectPropertyDefaults(
            const std::string& rootTypeId,
            const json& defaultsObject,
            const std::string& propertyPath);

        bool ProcessObjectRef(
            const std::string& rootTypeId,
            const std::string& typeStr,
            const std::string& propertyPath);

        bool ProcessEmbeddedObject(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& typeStr,
            const std::string& propertyPath);

        bool ProcessArray(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& propertyPath);

        bool ProcessValueProperty(
            const std::string& rootTypeId,
            const json& propValue,
            ELDSValueType valueType,
            const std::string& propertyPath);

        static bool IsValidEnumUnderlyingType(ELDSValueType valueType);

        bool ProcessEnumProperty(const std::string& rootTypeId, const json& jsonObject, const std::string& propertyPath);

        bool ProcessEnumPropertyItem(
            const std::string& rootTypeId,
            const json& itemJson,
            const std::string& itemPath,
            uint32 itemIndex,
            TOptional<ELDSValueType> underlyingValueType);

        bool ProcessEnumFlagsProperty(
            const std::string& rootTypeId,
            const json& jsonObject,
            const std::string& propertyPath);
    };
}