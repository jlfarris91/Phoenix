
#include "LDSJson.h"

#include "JsonCatalogTypeBuilder.h"
#include "JsonCatalogObjectBuilder.h"
#include "Platform.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

using json = nlohmann::json;

void Test_RegisterType_PodType()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "UInt32"
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::UInt32);
}

void Test_RegisterType_PodType_WithDefault()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "UInt32",
        "default": "FFFFFFFF"
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/default"_n) == 0xFFFFFFFF);
}

void Test_RegisterType_ObjectType_FailsWhenPropertiesIsEmpty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object"
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(!success);
    // TODO (jfarris): We should remove all records when a type fails to be added
    // PHX_ASSERT(!catalog.HasType("TestType"_n));
}

void Test_RegisterType_ObjectType_WithOnePodTypeProperty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32"
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testInt/type"_n) == ELDSValueType::Int32);
}

void Test_RegisterType_ObjectType_WithOneOfEveryPodTypeProperty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testBool": {
                "type": "Bool",
                "default": true
            },
            "testInt": {
                "type": "Int32",
                "default": -123
            },
            "testUInt": {
                "type": "UInt32",
                "default": 123
            },
            "testName": {
                "type": "Name",
                "default": "Foobar"
            },
            "testValue": {
                "type": "Value",
                "default": 1.23
            },
            "testDistance": {
                "type": "Distance",
                "default": 1.23
            },
            "testDegrees": {
                "type": "Degrees",
                "default": 1.23
            },
            "testSpeed": {
                "type": "Speed",
                "default": 1.23
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::Object);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testBool/type"_n) == ELDSValueType::Bool);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<bool>("TestType"_n, "/testBool/default"_n) == true);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testInt/default"_n) == -123);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testUInt/type"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testUInt/default"_n) == 123);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testName/type"_n) == ELDSValueType::Name);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testName/default"_n) == "Foobar"_n);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testValue/type"_n) == ELDSValueType::Value);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Value>("TestType"_n, "/testValue/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testDistance/type"_n) == ELDSValueType::Distance);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Distance>("TestType"_n, "/testDistance/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testDegrees/type"_n) == ELDSValueType::Degrees);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Angle>("TestType"_n, "/testDegrees/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testSpeed/type"_n) == ELDSValueType::Speed);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Speed>("TestType"_n, "/testSpeed/default"_n) == 1.23);
}

void Test_RegisterType_ObjectType_WithOneInlineObjectProperty_FailsWhenPropertiesIsEmpty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectInline": {
                "type": "Object"
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(!success);
    // TODO (jfarris): We should remove all records when a type fails to be added
    // PHX_ASSERT(!catalog.HasType("TestType"_n));
}

void Test_RegisterType_ObjectType_WithOneInlineObjectProperty_WithOnePodTypeProperty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectInline": {
                "type": "Object",
                "properties": {
                    "testInt": {
                        "type": "Int32",
                        "default": 100
                    }
                }
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::Object);

    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testObjectInline/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testObjectInline/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testObjectInline/testInt/default"_n) == 100);
}

void Test_RegisterType_ObjectType_WithOneInlineObjectRefProperty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testRefTypeJson = R"(
    {
        "id": "RefType",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32",
                "default": 100
            }
        }
    }
    )"_json;

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testInlineObjectRef": {
                "type": "Object#RefType"
            }
        }
    }
    )"_json;

    bool successfullyAddedRefType = typeBuilder.RegisterType(testRefTypeJson);
    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(successfullyAddedRefType);
    PHX_ASSERT(successfullyAddedTestType);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::Object);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testInlineObjectRef/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testInlineObjectRef/type"_n) == "RefType"_n);

    // TODO (jfarris): we need to copy in the properties of the referenced type
    // PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testInlineObjectRef/testInt/type"_n) == ELDSValueType::Int32);
    // PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testInlineObjectRef/testInt/default"_n) == 100);
}

void Test_RegisterType_ObjectType_WithOneObjectRefProperty_FailsWhenRefTypeIsNotValid()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectRef": {
                "type": "ObjectRef#RefType"
            }
        }
    }
    )"_json;

    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(!successfullyAddedTestType);
    // TODO (jfarris): We should remove all records when a type fails to be added
    // PHX_ASSERT(!catalog.HasType("TestType"_n));
}

void Test_RegisterType_ObjectType_WithOneObjectRefProperty()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);

    json testRefTypeJson = R"(
    {
        "id": "RefType",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32",
                "default": 100
            }
        }
    }
    )"_json;

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectRef": {
                "type": "ObjectRef#RefType"
            }
        }
    }
    )"_json;

    bool successfullyAddedRefType = typeBuilder.RegisterType(testRefTypeJson);
    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(successfullyAddedRefType);
    PHX_ASSERT(successfullyAddedTestType);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/type"_n) == ELDSValueType::Object);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testObjectRef/type"_n) == ELDSValueType::ObjectRef);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testObjectRef/type"_n) == "RefType"_n);
}

void Test_IntegrationTest()
{
    TCatalog<RecordStore, RecordStore> catalog;
    JsonCatalogTypeBuilder typeBuilder(&catalog);
    JsonCatalogObjectBuilder objectBuilder(&catalog);

    json colorJson = R"(
    {
        "id": "Color",
        "type": "UInt32",
        "format": "((?:[A-F]|[0-9])(?:[A-F]|[0-9])){3,4}",
        "min_len": 6,
        "max_len": 8,
        "default": "FFFFFFFF"
    }
    )"_json;

    json weaponJson = R"(
    {
        "id": "Weapon",
        "type": "Object",
        "properties": {
            "damage": {
                "type": "Int32",
                "default": 10,
                "min": 1,
                "max": 100
            },
            "range": {
                "type": "Distance",
                "default": 5.0
            }
        }
    }
    )"_json;

    json unitJson = R"(
    {
        "id": "Unit",
        "type": "Object",
        "properties": {
            "health": {
                "type": "Int32",
                "default": 100
            },
            "weapon": {
                "type": "ObjectRef#Weapon",
                "default": null
            }
        }
    }
    )"_json;

    TArray2<json> types;
    types.EmplaceBack(colorJson);
    types.EmplaceBack(weaponJson);
    types.EmplaceBack(unitJson);

    for (const auto& typeJson : types)
    {
        bool success = typeBuilder.RegisterType(typeJson);
        PHX_ASSERT(success);
    }

    json baseUnitJson = R"(
    {
        "id": "BaseUnit",
        "base": "Unit",
        "health": 456,
        "weapon": "BaseWeapon"
    }
    )"_json;

    json lancerJson = R"(
    {
        "id": "Lancer",
        "base": "BaseUnit",
        "health": 123,
        "weapon": "LancerWeapon"
    }
    )"_json;

    json baseWeaponJson = R"(
    {
        "id": "BaseWeapon",
        "base": "Weapon",
        "damage": 100
    }
    )"_json;

    json lancerWeaponJson = R"(
    {
        "id": "LancerWeapon",
        "base": "BaseWeapon",
        "damage": 75
    }
    )"_json;

    TArray2<json> objects;
    objects.EmplaceBack(baseUnitJson);
    objects.EmplaceBack(baseWeaponJson);
    objects.EmplaceBack(lancerJson);
    objects.EmplaceBack(lancerWeaponJson);

    for (const auto& objectJson : objects)
    {
        bool success = objectBuilder.RegisterObject(objectJson);
        PHX_ASSERT(success);
    }

    LDSRecord* record = catalog.FindObjectRecord("Lancer"_n, "/health"_n);
    PHX_ASSERT(record);
    PHX_ASSERT(record->GetValue().Type == ELDSValueType::Int32);
    PHX_ASSERT(record->GetValue().Value.Int32 == 123);
    PHX_ASSERT(record->GetValueAs<int32>() == 123);
}

void Json::Test()
{
    Test_RegisterType_PodType();
    Test_RegisterType_PodType_WithDefault();

    Test_RegisterType_ObjectType_FailsWhenPropertiesIsEmpty();

    Test_RegisterType_ObjectType_WithOnePodTypeProperty();
    Test_RegisterType_ObjectType_WithOneOfEveryPodTypeProperty();

    Test_RegisterType_ObjectType_WithOneInlineObjectProperty_FailsWhenPropertiesIsEmpty();
    Test_RegisterType_ObjectType_WithOneInlineObjectProperty_WithOnePodTypeProperty();
    Test_RegisterType_ObjectType_WithOneInlineObjectRefProperty();

    Test_RegisterType_ObjectType_WithOneObjectRefProperty_FailsWhenRefTypeIsNotValid();
    Test_RegisterType_ObjectType_WithOneObjectRefProperty();

    Test_IntegrationTest();
}
