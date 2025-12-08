
#include "Json/LDSJsonTests.h"

#include "LDSRecordView.h"
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

    auto warnings = typeBuilder.GetLogs(ELogLevel::Warning);
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

    auto warnings = typeBuilder.GetLogs(ELogLevel::Warning);
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

    auto warnings = typeBuilder.GetLogs(ELogLevel::Warning);
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

    auto warnings = typeBuilder.GetLogs(ELogLevel::Warning);
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

void Test_RegisterType_ObjectType_WithEnumProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testEnum": {
                "type": "Enum",
                "items": [
                    "TestEnumValue0",
                    "TestEnumValue1",
                    "TestEnumValue2"
                ]
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testEnum/type"_n) == ELDSValueType::Enum);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testEnum/underlying_type"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testEnum/items/size"_n) == 3);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/0/key"_n) == "TestEnumValue0"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testEnum/items/0/value"_n) == 0);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/1/key"_n) == "TestEnumValue1"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testEnum/items/1/value"_n) == 1);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/2/key"_n) == "TestEnumValue2"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testEnum/items/2/value"_n) == 2);
}

void Test_RegisterType_ObjectType_WithEnumProperty_WithUnderlyingType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testEnum": {
                "type": "Enum",
                "items": [
                    { "TestEnumValue0": 1.11 },
                    { "TestEnumValue1": 2.22 },
                    { "TestEnumValue2": 3.33 }
                ],
                "underlying_type": "Distance"
            }
        }
    }
    )"_json;

    bool success = typeBuilder.RegisterType(testTypeJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/testEnum/type"_n) == ELDSValueType::Enum);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<ELDSValueType>("TestType"_n, "/testEnum/underlying_type"_n) == ELDSValueType::Distance);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/testEnum/items/size"_n) == 3);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/0/key"_n) == "TestEnumValue0"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Distance>("TestType"_n, "/testEnum/items/0/value"_n) == 1.11);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/1/key"_n) == "TestEnumValue1"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Distance>("TestType"_n, "/testEnum/items/1/value"_n) == 2.22);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<FName>("TestType"_n, "/testEnum/items/2/key"_n) == "TestEnumValue2"_n);
    PHX_ASSERT(catalog.GetTypeRecordValueAs<Distance>("TestType"_n, "/testEnum/items/2/value"_n) == 3.33);
}

void Test_RegisterType_ObjectType_WithArrayProperty_WithPodItemType()
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

void Test_RegisterType_ObjectType_WithArrayProperty_WithInlineObjectItemType()
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

void Test_RegisterType_ObjectType_WithArrayProperty_WithEmbeddedObjectItemType()
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

void Test_RegisterType_ObjectType_WithArrayProperty_WithObjectRefItemType()
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

void Test_RegisterObject_FailsWhenNoIdPropertyIsDefined()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Int32"
    }
    )"_json;

    json testObjectJson = R"(
    {
        "base": "TestType"
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(!success);
    auto errors = objectBuilder.GetLogs(ELogLevel::Error);
    PHX_ASSERT(errors.size() == 1);
    PHX_ASSERT(errors.back().Id.empty());
    PHX_ASSERT(errors.back().PropertyPath.empty());
    PHX_ASSERT(errors.back().Message == "Object is missing required 'id' property.");
}

void Test_RegisterObject_FailsWhenNoBasePropertyIsDefined()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Int32"
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestObject"
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(!success);
    auto errors = objectBuilder.GetLogs(ELogLevel::Error);
    PHX_ASSERT(errors.size() == 1);
    PHX_ASSERT(errors.back().Id == "TestObject");
    PHX_ASSERT(errors.back().PropertyPath.empty());
    PHX_ASSERT(errors.back().Message == "Object is missing required 'base' property.");
}

void Test_RegisterObject_FailsWhenObjectWithSameIdIsAlreadyRegistered()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object"
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType"
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    objectBuilder.RegisterObject(testObjectJson);
    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(!success);
    auto errors = objectBuilder.GetLogs(ELogLevel::Error);
    PHX_ASSERT(errors.size() == 1);
    PHX_ASSERT(errors.back().Id == "TestObject");
    PHX_ASSERT(errors.back().PropertyPath.empty());
    PHX_ASSERT(errors.back().Message == "Object with id 'TestObject' has already been registered.");
}

void Test_RegisterObject_FailsWhenBaseIsNotRegistered()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestBaseObject"
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(!success);
    auto errors = objectBuilder.GetLogs(ELogLevel::Error);
    PHX_ASSERT(errors.size() == 1);
    PHX_ASSERT(errors.back().Id == "TestObject");
    PHX_ASSERT(errors.back().PropertyPath.empty());
    PHX_ASSERT(errors.back().Message == "No base object or type registered with id 'TestBaseObject'.");
}

void Test_RegisterObject_FailsWhenNonObjectType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Int32"
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestPOD",
        "base": "TestType"
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(!success);
    auto errors = objectBuilder.GetLogs(ELogLevel::Error);
    PHX_ASSERT(errors.size() == 1);
    PHX_ASSERT(errors.back().Id == "TestPOD");
    PHX_ASSERT(errors.back().PropertyPath.empty());
    PHX_ASSERT(errors.back().Message == "Root objects must be of type Object.");
}

void Test_RegisterObject()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object"
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType"
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.HasObject("TestObject"_n));
    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/base"_n) == ELDSValueType::Name);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/base"_n) == "TestType"_n);
}

void Test_RegisterObject_WithPodTypeProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testInt": 100
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testInt"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testInt"_n) == 100);
}

void Test_RegisterObject_WithOneOfEveryPodTypeProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testBool": true,
        "testInt": -123,
        "testUInt": 123,
        "testName": "Foobar",
        "testValue": 1.23,
        "testDistance": 1.23,
        "testDegrees": 1.23,
        "testSpeed": 1.23
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testBool"_n) == ELDSValueType::Bool);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<bool>("TestObject"_n, "/testBool"_n) == true);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testInt"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testInt"_n) == -123);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testUInt"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testUInt"_n) == 123);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testName"_n) == ELDSValueType::Name);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/testName"_n) == "Foobar"_n);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testValue"_n) == ELDSValueType::Value);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<Value>("TestObject"_n, "/testValue"_n) == 1.23);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testDistance"_n) == ELDSValueType::Distance);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<Distance>("TestObject"_n, "/testDistance"_n) == 1.23);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testDegrees"_n) == ELDSValueType::Degrees);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<Angle>("TestObject"_n, "/testDegrees"_n) == 1.23);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testSpeed"_n) == ELDSValueType::Speed);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<Speed>("TestObject"_n, "/testSpeed"_n) == 1.23);
}

void Test_RegisterObject_WithInlineObjectProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testObjectInline": {
                "type": "Object",
                "properties": {
                    "testInt": {
                        "type": "Int32"
                    }
                }
            }
        }
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testObjectInline": {
            "testInt": 100
        }
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testObjectInline/testInt"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testObjectInline/testInt"_n) == 100);
}

void Test_RegisterObject_WithEmbeddedObjectProperty()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);
    JsonCatalogObjectBuilder objectBuilder(&dataSource, &catalog);

    json testRefTypeJson = R"(
    {
        "id": "RefType",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32"
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
    typeBuilder.RegisterAllTypes();

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testEmbeddedObject": {
            "testInt": 100
        }
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testEmbeddedObject/testInt"_n) == ELDSValueType::Int32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testEmbeddedObject/testInt"_n) == 100);
}

void Test_RegisterObject_WithObjectRefProperty()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);
    JsonCatalogObjectBuilder objectBuilder(&dataSource, &catalog);

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
    typeBuilder.RegisterAllTypes();

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testObjectRef": "TestRefTypeObject"
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);
    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testObjectRef"_n) == ELDSValueType::ObjectRef);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/testObjectRef"_n) == "TestRefTypeObject"_n);
}

void Test_RegisterObject_WithArrayProperty_WithPodItemType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    typeBuilder.RegisterType(testTypeJson);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testArray": [ 123, 456, 789 ]
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testArray/size"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testArray/size"_n) == 3);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/0"_n) == 123);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/1"_n) == 456);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/2"_n) == 789);
}

void Test_RegisterObject_WithArrayProperty_WithInlineObjectItemType()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    typeBuilder.RegisterType(testTypeJson);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testArray": [
            { "testInt": 123 },
            { "testInt": 456 },
            { "testInt": 789 }
        ]
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testArray/size"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testArray/size"_n) == 3);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/0/testInt"_n) == 123);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/1/testInt"_n) == 456);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/2/testInt"_n) == 789);
}

void Test_RegisterObject_WithArrayProperty_WithEmbeddedObjectItemType()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);
    JsonCatalogObjectBuilder objectBuilder(&dataSource, &catalog);

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
    typeBuilder.RegisterAllTypes();

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testArray": [
            { "testInt": 123 },
            { "testInt": 456 },
            { "testInt": 789 }
        ]
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testArray/size"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testArray/size"_n) == 3);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/0/testInt"_n) == 123);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/1/testInt"_n) == 456);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/2/testInt"_n) == 789);
}

void Test_RegisterObject_WithArrayProperty_WithObjectRefItemType()
{
    JsonDataSource dataSource;
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(&dataSource, &catalog);
    JsonCatalogObjectBuilder objectBuilder(&dataSource, &catalog);

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
    typeBuilder.RegisterAllTypes();

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testArray": [
            "SomeRefTypeObject0",
            "SomeRefTypeObject1",
            "SomeRefTypeObject2"
        ]
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testArray/size"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testArray/size"_n) == 3);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/testArray/0"_n) == "SomeRefTypeObject0"_n);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/testArray/1"_n) == "SomeRefTypeObject1"_n);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<FName>("TestObject"_n, "/testArray/2"_n) == "SomeRefTypeObject2"_n);
}

void Test_RegisterObject_WithArrayProperty_WithMaxItems()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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
                "max_items": 2
            }
        }
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testArray": [ 123, 456, 789 ]
    }
    )"_json;

    bool success = objectBuilder.RegisterObject(testObjectJson);

    PHX_ASSERT(success);

    PHX_ASSERT(catalog.GetObjectRecordValueType("TestObject"_n, "/testArray/size"_n) == ELDSValueType::UInt32);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<uint32>("TestObject"_n, "/testArray/size"_n) == 2);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/0"_n) == 123);
    PHX_ASSERT(catalog.GetObjectRecordValueAs<int32>("TestObject"_n, "/testArray/1"_n) == 456);
}

void Test_FindObjectRecord_ValueDefinedInObject_WithPodProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    typeBuilder.RegisterType(testTypeJson);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType",
        "testInt": 123
    }
    )"_json;

    objectBuilder.RegisterObject(testObjectJson);

    // Act
    const LDSRecord* objectRecord = catalog.FindObjectRecord("TestObject"_n, "/testInt"_n);

    PHX_ASSERT(objectRecord);
}

void Test_FindObjectRecord_ValueDefinedInBaseObject_WithPodProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    typeBuilder.RegisterType(testTypeJson);

    json testBaseObjectJson = R"(
    {
        "id": "TestBaseObject",
        "base": "TestType",
        "testInt": 123
    }
    )"_json;

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestBaseObject"
    }
    )"_json;

    objectBuilder.RegisterObject(testBaseObjectJson);
    objectBuilder.RegisterObject(testObjectJson);

    const LDSRecord* baseObjectRecord = catalog.FindObjectRecord("TestBaseObject"_n, "/testInt"_n);

    // Act
    const LDSRecord* objectRecord = catalog.FindObjectRecord("TestObject"_n, "/testInt"_n);

    PHX_ASSERT(objectRecord);
    PHX_ASSERT(objectRecord == baseObjectRecord);
}

void Test_FindObjectRecord_OnlyDefaultDefinedInBaseType_WithPodProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

    json testTypeJson = R"(
    {
        "id": "TestType",
        "type": "Object",
        "properties": {
            "testInt": {
                "type": "Int32",
                "default": 123
            }
        }
    }
    )"_json;

    typeBuilder.RegisterType(testTypeJson);

    const LDSRecord* defaultRecord = catalog.FindTypeRecord("TestType"_n, "/testInt/default"_n);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType"
    }
    )"_json;

    objectBuilder.RegisterObject(testObjectJson);

    // Act
    const LDSRecord* objectRecord = catalog.FindObjectRecord("TestObject"_n, "/testInt"_n);

    PHX_ASSERT(objectRecord);
    PHX_ASSERT(objectRecord == defaultRecord);
}

void Test_FindObjectRecord_NoDefaultDefinedInBaseType_WithPodProperty()
{
    LDSCatalog catalog;
    JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);
    JsonCatalogObjectBuilder objectBuilder(nullptr, &catalog);

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

    typeBuilder.RegisterType(testTypeJson);

    json testObjectJson = R"(
    {
        "id": "TestObject",
        "base": "TestType"
    }
    )"_json;

    objectBuilder.RegisterObject(testObjectJson);

    // Act
    const LDSRecord* objectRecord = catalog.FindObjectRecord("TestObject"_n, "/testInt"_n);

    PHX_ASSERT(!objectRecord);
}

void Test_IntegrationTest();

void Json::RunLDSJsonTests()
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

    Test_RegisterType_ObjectType_WithEnumProperty();
    Test_RegisterType_ObjectType_WithEnumProperty_WithUnderlyingType();

    Test_RegisterType_ObjectType_WithArrayProperty_WithPodItemType();
    Test_RegisterType_ObjectType_WithArrayProperty_WithInlineObjectItemType();
    Test_RegisterType_ObjectType_WithArrayProperty_WithEmbeddedObjectItemType();
    Test_RegisterType_ObjectType_WithArrayProperty_WithObjectRefItemType();
    Test_RegisterType_ObjectType_WithArrayProperty_WithMinMaxItems();

    Test_RegisterObject_FailsWhenNoIdPropertyIsDefined();
    Test_RegisterObject_FailsWhenNoBasePropertyIsDefined();
    Test_RegisterObject_FailsWhenObjectWithSameIdIsAlreadyRegistered();
    Test_RegisterObject_FailsWhenBaseIsNotRegistered();
    Test_RegisterObject_FailsWhenNonObjectType();

    Test_RegisterObject();
    Test_RegisterObject_WithPodTypeProperty();
    Test_RegisterObject_WithOneOfEveryPodTypeProperty();
    Test_RegisterObject_WithInlineObjectProperty();
    Test_RegisterObject_WithEmbeddedObjectProperty();
    Test_RegisterObject_WithObjectRefProperty();
    Test_RegisterObject_WithArrayProperty_WithPodItemType();
    Test_RegisterObject_WithArrayProperty_WithInlineObjectItemType();
    Test_RegisterObject_WithArrayProperty_WithEmbeddedObjectItemType();
    Test_RegisterObject_WithArrayProperty_WithObjectRefItemType();
    Test_RegisterObject_WithArrayProperty_WithMaxItems();

    Test_FindObjectRecord_ValueDefinedInObject_WithPodProperty();
    Test_FindObjectRecord_ValueDefinedInBaseObject_WithPodProperty();
    Test_FindObjectRecord_OnlyDefaultDefinedInBaseType_WithPodProperty();
    Test_FindObjectRecord_NoDefaultDefinedInBaseType_WithPodProperty();

    Test_IntegrationTest();
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
        auto itemRecord = weaponsArray.ItemRecord(0);
        PHX_ASSERT(itemRecord);
        PHX_ASSERT(itemRecord->GetValueType() == ELDSValueType::ObjectRef);
        PHX_ASSERT(itemRecord->GetValueAs<FName>() == "LancerWeapon"_n);
        PHX_ASSERT(weaponsArray.ItemValueAs<FName>(0) == "LancerWeapon"_n);
    }

    {
        TLDSObjectView lancer(&catalog, "Lancer"_n);
        uint32 lancerWeaponDamage;

        lancerWeaponDamage = lancer.Array("/weapons").ItemAsResolvedObject(0).PropertyAs<uint32>("/damage");
        PHX_ASSERT(lancerWeaponDamage == 75);

        lancerWeaponDamage = (lancer / "/weapons" / 0 / Resolve / "/damage").As<uint32>();
        PHX_ASSERT(lancerWeaponDamage == 75);
    }
}