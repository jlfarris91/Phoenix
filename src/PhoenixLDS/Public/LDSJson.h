
#pragma once

#include "DLLExport.h"
#include "FixedLDS.h"

namespace Phoenix::LDS::Json
{
    using json = nlohmann::json;

    PHOENIX_LDS_API bool FromJson(
        const json& json,
        ELDSValueType type,
        LDSValue& outValue);

    PHOENIX_LDS_API bool FromJson(
        const TFixedLDS<64>& lds,
        const TFixedLDS<64>& metadata,
        const json& json,
        const FName& archetypeId,
        const char* pointerStr,
        uint32 pointerFirst,
        uint32 pointerLast,
        LDSTypedValue& outValue);

    PHOENIX_LDS_API bool FromJson(
        const TFixedLDS<64>& lds,
        const TFixedLDS<64>& metadata,
        const json& json,
        const FName& archetypeId,
        const json::json_pointer& pointer,
        LDSTypedValue& outValue);

    PHOENIX_LDS_API bool ProcessTypeRoot(TFixedLDS<64>& metadata, const json& typeJson);

    PHOENIX_LDS_API bool ProcessObject(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const FName& objectId,
        const json& jsonObject,
        const PHXString& propertyPath);

    PHOENIX_LDS_API bool ProcessObjectRef(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const FName& objectId,
        const json& json,
        const PHXString& typeStr,
        const PHXString& propertyPath);

    PHOENIX_LDS_API bool ProcessInlineObjectRef(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const FName& objectId,
        const json& jsonObject,
        const PHXString& typeStr,
        const PHXString& propertyPath);

    PHOENIX_LDS_API bool ProcessObject(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const FName& objectId,
        const PHXString& key,
        const json& jsonObject,
        const PHXString& propertyPath);

    PHOENIX_LDS_API bool ProcessTypeObjectProperty(
        TFixedLDS<64>& metadata,
        const FName& typeId,
        const PHXString& propertyPath,
        const PHXString& propName,
        const json& propValue);

    PHOENIX_LDS_API bool ProcessArray(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const FName& objectId,
        const json& jsonObject,
        const PHXString& propertyPath);

    PHOENIX_LDS_API bool ProcessPODProperty(
        TFixedLDS<64>& metadata,
        const FName& rootObjectId,
        const PHXString& propName,
        const json& propValue,
        ELDSValueType valueType,
        const PHXString& propertyPath);

    PHOENIX_LDS_API void Test();
}
