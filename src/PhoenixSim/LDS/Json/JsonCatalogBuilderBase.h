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
        std::string Id;
        std::string PropertyPath;

        JsonCatalogBuilderLogMessage& Context(const std::string& id, const std::string& path = {})
        {
            Id = id;
            PropertyPath = path;
            return *this;
        }
    };

    struct PHOENIX_SIM_API JsonCatalogBuilderBase : ILogCollectionOwner<JsonCatalogBuilderLogMessage>
    {
        using json = nlohmann::json;

        JsonCatalogBuilderBase(const JsonDataSource* dataSource, HeapLDSCatalog* catalog);

    protected:

        static bool GetValueFromJson(
            const json& json,
            ELDSValueType type,
            LDSValue& outValue);

        bool GetPropertyValueObjectRefFromJson(const json& json, LDSValue& value);

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const std::string& propertyPath,
            uint32 pointerFirst,
            uint32 pointerLast,
            LDSTypedValue& outValue);

        bool GetPropertyValueFromJson(
            const json& json,
            const FName& objectId,
            const std::string& propertyPath,
            LDSTypedValue& outValue);

        HeapLDSCatalog* Catalog;
        const JsonDataSource* DataSource;
        LDSCatalogQueryContext ObjectQueryContext;
        LDSCatalogQueryContext TypeQueryContext;
    };
}
