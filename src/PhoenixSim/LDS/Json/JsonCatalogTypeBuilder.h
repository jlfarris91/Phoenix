
#pragma once

#include "PhoenixSim/LDS/Json/JsonCatalogBuilderBase.h"

namespace Phoenix::LDS::Json
{
    template <class TCatalog = Catalog>
    struct JsonCatalogTypeBuilder : JsonCatalogBuilderBase<TCatalog>
    {
        using json = nlohmann::json;

        JsonCatalogTypeBuilder(const JsonDataSource* dataSource, TCatalog* catalog);

        bool RegisterAllTypes();

        bool RegisterType(const json& typeJson);

    private:

        bool ProcessJsonObject(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath);

        bool ProcessObject(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath);

        bool ProcessObjectProperties(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath);

        bool ProcessObjectPropertyDefaults(
            const PHXString& rootTypeId,
            const json& defaultsObject,
            const PHXString& propertyPath);

        bool ProcessObjectRef(
            const PHXString& rootTypeId,
            const PHXString& typeStr,
            const PHXString& propertyPath);

        bool ProcessEmbeddedObject(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& typeStr,
            const PHXString& propertyPath);

        bool ProcessArray(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath);

        bool ProcessValueProperty(
            const PHXString& rootTypeId,
            const json& propValue,
            ELDSValueType valueType,
            const PHXString& propertyPath);

        bool IsValidEnumUnderlyingType(ELDSValueType valueType);

        bool ProcessEnumProperty(const PHXString& rootTypeId, const json& jsonObject, const PHXString& propertyPath);

        bool ProcessEnumPropertyItem(
            const PHXString& rootTypeId,
            const json& itemJson,
            const PHXString& itemPath,
            uint32 itemIndex,
            TOptional<ELDSValueType> underlyingValueType);

        bool ProcessEnumFlagsProperty(
            const PHXString& rootTypeId,
            const json& jsonObject,
            const PHXString& propertyPath);
    };
}

#include "PhoenixSim/LDS/Json/JsonCatalogTypeBuilder.inl"