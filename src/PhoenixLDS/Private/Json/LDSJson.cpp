
#include "Json/LDSJson.h"

#include "LDSArrayView.h"
#include "Json/JsonCatalogTypeBuilder.h"
#include "Json/JsonCatalogObjectBuilder.h"
#include "Platform.h"
#include "Json/JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

using json = nlohmann::json;
using LDSCatalog = TLDSCatalog<RecordStore, RecordStore>;

void Test_RegisterType_PodType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/type"_n) == ELDSValueType::UInt32);
}

void Test_RegisterType_PodType_WithDefault()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/type"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/default"_n) == 0xFFFFFFFF);
}

void Test_RegisterType_ObjectType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {}
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasType("TestType"_n));
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/id"_n) == "TestType"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/type"_n) == ELDSValueType::Object);
}

void Test_RegisterType_ObjectType_WarnsWhenPropertiesIsMissing()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object"
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    auto warnings = typeBuilder.GetLogs(EJsonCatalogBuilderLogLevel::Warning);
    PHX_ASSERT(warnings.size() == 1);
    PHX_ASSERT(warnings.back().Id == "TestType");
    PHX_ASSERT(warnings.back().PropertyPath.empty());
    PHX_ASSERT(warnings.back().Message == "Object type is missing expected 'properties' property.");
}

void Test_RegisterType_ObjectType_WarnsWhenPropertiesIsEmpty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {}
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    auto warnings = typeBuilder.GetLogs(EJsonCatalogBuilderLogLevel::Warning);
    PHX_ASSERT(warnings.size() == 1);
    PHX_ASSERT(warnings.back().Id == "TestType");
    PHX_ASSERT(warnings.back().PropertyPath.empty());
    PHX_ASSERT(warnings.back().Message == "Object type 'properties' is empty.");
}

void Test_RegisterType_ObjectType_WithPodTypeProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testInt/type"_n) == ELDSValueType::Int32);
}

void Test_RegisterType_ObjectType_WithOneOfEveryPodTypeProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testBool/type"_n) == ELDSValueType::Bool);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<bool>("TestType"_n, "/testBool/default"_n) == true);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testInt/default"_n) == -123);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testUInt/type"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testUInt/default"_n) == 123);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testName/type"_n) == ELDSValueType::Name);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testName/default"_n) == "Foobar"_n);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testValue/type"_n) == ELDSValueType::Value);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Value>("TestType"_n, "/testValue/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testDistance/type"_n) == ELDSValueType::Distance);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Distance>("TestType"_n, "/testDistance/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testDegrees/type"_n) == ELDSValueType::Degrees);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Angle>("TestType"_n, "/testDegrees/default"_n) == 1.23);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testSpeed/type"_n) == ELDSValueType::Speed);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Speed>("TestType"_n, "/testSpeed/default"_n) == 1.23);
}

void Test_RegisterType_ObjectType_WithInlineObjectProperty_WarnsWhenPropertiesIsMissing()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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

    PHX_ASSERT(success);

    auto warnings = typeBuilder.GetLogs(EJsonCatalogBuilderLogLevel::Warning);
    PHX_ASSERT(warnings.size() == 1);
    PHX_ASSERT(warnings.back().Id == "TestType");
    PHX_ASSERT(warnings.back().PropertyPath == "/testObjectInline");
    PHX_ASSERT(warnings.back().Message == "Object type is missing expected 'properties' property.");
}

void Test_RegisterType_ObjectType_WithInlineObjectProperty_WarnsWhenPropertiesIsEmpty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectInline": {
                "type": "Object",
                "properties": {}
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    auto warnings = typeBuilder.GetLogs(EJsonCatalogBuilderLogLevel::Warning);
    PHX_ASSERT(warnings.size() == 1);
    PHX_ASSERT(warnings.back().Id == "TestType");
    PHX_ASSERT(warnings.back().PropertyPath == "/testObjectInline");
    PHX_ASSERT(warnings.back().Message == "Object type 'properties' is empty.");
}

void Test_RegisterType_ObjectType_WithInlineObjectProperty_WithOnePodTypeProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

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

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testObjectInline/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testObjectInline/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testObjectInline/testInt/default"_n) == 100);
}

void Test_RegisterType_ObjectType_WithEmbeddedObjectProperty()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);

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
            "testEmbeddedObject": {
                "type": "Object#RefType"
            }
        }
    }
    )"_json;

    dataSource.RegisterType(testRefTypeJson);
    dataSource.RegisterType(testTypeJson);

    bool successfullyAddedRefType = typeBuilder.RegisterType(testRefTypeJson);
    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(successfullyAddedRefType);
    PHX_ASSERT(successfullyAddedTestType);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testEmbeddedObject/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testEmbeddedObject/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testEmbeddedObject/testInt/default"_n) == 100);
}

void Test_RegisterType_ObjectType_WithObjectRefProperty()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);

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

    dataSource.RegisterType(testRefTypeJson);
    dataSource.RegisterType(testTypeJson);

    bool successfullyAddedRefType = typeBuilder.RegisterType(testRefTypeJson);
    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(successfullyAddedRefType);
    PHX_ASSERT(successfullyAddedTestType);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testObjectRef/type"_n) == ELDSValueType::ObjectRef);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testObjectRef/type"_n) == "RefType"_n);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithPodType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "Int32",
                    "default": 10
                }
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/type"_n) == ELDSValueType::Array);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testArray/items/default"_n) == 10);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithInlineObject()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "Object",
                    "properties": {
                        "testInt": {
                            "type": "Int32",
                            "default": 123
                        }
                    }
                }
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/type"_n) == ELDSValueType::Array);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testArray/items/testInt/default"_n) == 123);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithEmbeddedObject()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);

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
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "Object#RefType"
                }
            }
        }
    }
    )"_json;

    dataSource.RegisterType(testRefTypeJson);
    dataSource.RegisterType(testTypeJson);

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/type"_n) == ELDSValueType::Array);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/type"_n) == ELDSValueType::Object);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/testInt/type"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testArray/items/testInt/default"_n) == 100);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithObjectRef()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);

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
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "ObjectRef#RefType"
                }
            }
        }
    }
    )"_json;
    
    dataSource.RegisterType(testRefTypeJson);
    dataSource.RegisterType(testTypeJson);

    bool successfullyAddedRefType = typeBuilder.RegisterType(testRefTypeJson);
    bool successfullyAddedTestType = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(successfullyAddedRefType);
    PHX_ASSERT(successfullyAddedTestType);

    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/type"_n) == ELDSValueType::Array);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testArray/items/type"_n) == ELDSValueType::ObjectRef);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testArray/items/type"_n) == "RefType"_n);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithMinMaxItems()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testArray": {
                "type": "Array",
                "items": {
                    "type": "Int32",
                    "default": 10
                },
                "min_items": 1,
                "max_items": 5
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testArray/min_items"_n) == 1);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<int32>("TestType"_n, "/testArray/max_items"_n) == 5);
}

void Test_IntegrationTest()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);
    JsonCatalogObjectBuilder objectBuilder(&dataSource, &catalog);

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
            "weapons": {
                "type": "Array",
                "items": {
                    "type": "ObjectRef#Weapon"
                }
            },
            "armor": {
                "type": "Object",
                "properties": {
                    "flat_reduc": {
                        "type": "Value"
                    },
                    "perc_reduc": {
                        "type": "Value"
                    },
                    "bonus": {
                        "type": "Value",
                        "default": 123.0
                    }
                }
            }
        }
    }
    )"_json;

    dataSource.RegisterType(colorJson);
    dataSource.RegisterType(weaponJson);
    dataSource.RegisterType(unitJson);

    typeBuilder.RegisterAllTypes();

    json baseUnitJson = R"(
    {
        "id": "BaseUnit",
        "base": "Unit",
        "health": 456,
        "weapons": [
            "BaseWeapon"
        ],
        "armor": {
            "flat_reduc": 10
        }
    }
    )"_json;

    json lancerJson = R"(
    {
        "id": "Lancer",
        "base": "BaseUnit",
        "health": 123,
        "weapons": [
            "LancerWeapon"
        ],
        "armor": {
            "perc_reduc": 50
        }
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

    dataSource.RegisterObject(baseUnitJson);
    dataSource.RegisterObject(baseWeaponJson);
    dataSource.RegisterObject(lancerJson);
    dataSource.RegisterObject(lancerWeaponJson);

    objectBuilder.RegisterAllObjects();

    {
        LDSRecord* record = catalog.FindObjectRecord("Lancer"_n, "/health"_n);
        PHX_ASSERT(record);
        PHX_ASSERT(record->GetValue().Type == ELDSValueType::Int32);
        PHX_ASSERT(record->GetValueAs<int32>() == 123);
    }

    {
        LDSRecord* record = catalog.FindObjectRecord("Lancer"_n, "/armor/flat_reduc"_n);
        PHX_ASSERT(record);
        PHX_ASSERT(record->GetValue().Type == ELDSValueType::Value);
        PHX_ASSERT(record->GetValueAs<Value>() == 10);
    }

    {
        LDSRecord* record = catalog.FindObjectRecord("Lancer"_n, "/armor/perc_reduc"_n);
        PHX_ASSERT(record);
        PHX_ASSERT(record->GetValue().Type == ELDSValueType::Value);
        PHX_ASSERT(record->GetValueAs<Value>() == 50);
    }

    {
        LDSRecord* record = catalog.FindObjectRecord("Lancer"_n, "/armor/bonus"_n);
        PHX_ASSERT(record);
        PHX_ASSERT(record->GetValue().Type == ELDSValueType::Value);
        PHX_ASSERT(record->GetValueAs<Value>() == 123);
    }

    {
        TLDSArrayView weaponsArray(&catalog, "Lancer"_n, "/weapons"_n);
        PHX_ASSERT(weaponsArray.IsValid());
        PHX_ASSERT(weaponsArray.Num() == 1);
        auto itemRecord = weaponsArray.GetItemRecord(0);
        PHX_ASSERT(itemRecord);
        PHX_ASSERT(itemRecord->GetValueType() == ELDSValueType::ObjectRef);
        PHX_ASSERT(itemRecord->GetValueAs<FName>() == "LancerWeapon"_n);
        PHX_ASSERT(weaponsArray.GetValueAs<FName>(0) == "LancerWeapon"_n);
    }
}

void Json::RunLDSTests()
{
    Test_RegisterType_PodType();
    Test_RegisterType_PodType_WithDefault();

    Test_RegisterType_ObjectType();
    Test_RegisterType_ObjectType_WarnsWhenPropertiesIsMissing();
    Test_RegisterType_ObjectType_WarnsWhenPropertiesIsEmpty();

    Test_RegisterType_ObjectType_WithPodTypeProperty();
    Test_RegisterType_ObjectType_WithOneOfEveryPodTypeProperty();

    Test_RegisterType_ObjectType_WithInlineObjectProperty_WarnsWhenPropertiesIsMissing();
    Test_RegisterType_ObjectType_WithInlineObjectProperty_WarnsWhenPropertiesIsEmpty();
    Test_RegisterType_ObjectType_WithInlineObjectProperty_WithOnePodTypeProperty();

    Test_RegisterType_ObjectType_WithEmbeddedObjectProperty();

    Test_RegisterType_ObjectType_WithObjectRefProperty();

    Test_RegisterType_ObjectType_WithArrayProperty_WithPodType();
    Test_RegisterType_ObjectType_WithArrayProperty_WithInlineObject();
    Test_RegisterType_ObjectType_WithArrayProperty_WithEmbeddedObject();
    Test_RegisterType_ObjectType_WithArrayProperty_WithObjectRef();
    Test_RegisterType_ObjectType_WithArrayProperty_WithMinMaxItems();

    Test_IntegrationTest();
}
