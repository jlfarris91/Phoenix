
#include <doctest/doctest.h>

#include "Phoenix.Sim/LDS/LDSCatalog.h"
#include "Phoenix.Sim/LDS/Json/JsonCatalogObjectBuilder.h"
#include "Phoenix.Sim/LDS/Json/JsonCatalogTypeBuilder.h"
#include "Phoenix.Sim/LDS/Json/JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::LDS::Json;

using json = nlohmann::json;

TEST_SUITE("JsonCatalogTypeBuilder")
{
    TEST_CASE("register pod type")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register pod type with default value")
    {
        HeapLDSCatalog catalog;
        JsonCatalogTypeBuilder typeBuilder(nullptr, &catalog);

        nlohmann::json testTypeJson = R"(
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
        PHX_ASSERT(catalog.GetTypeRecordValueType("TestType"_n, "/type"_n) == LDS::ELDSValueType::UInt32);
        PHX_ASSERT(catalog.GetTypeRecordValueAs<uint32>("TestType"_n, "/default"_n) == 0xFFFFFFFF);
    }

    TEST_CASE("register object type")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type warns when properties field is missing")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type warns when properties field is empty")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with pod type property")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with one of every pod type property")
    {
        HeapLDSCatalog catalog;
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
    
    TEST_CASE("register object type with inline object property warns that properties field is missing")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with inline object property with empty properties field warns that properties field is empty")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with inline object property with one pod property")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with embedded object property")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with object ref property")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with enum property")
    {
        HeapLDSCatalog catalog;
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
    
    TEST_CASE("register object type with enum property with underlying type and non-integer values")
    {
        HeapLDSCatalog catalog;
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
    
    TEST_CASE("register object type with array property containing pod items")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with array property containing inline object items")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with array property containing embedded object items")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with array property containing object ref items")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object type with array property containing pod items with metadata")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with missing id field fails")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with missing base field fails")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with non-existent base type fails")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with base type that is not an object fails")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with pod property")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with one of every pod property")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with inline object property with one pod property")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with embedded object property")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with object ref property")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with array property containing pod items")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with array property containing inline object items")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with array property containing embedded object items")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with array property containing object ref items")
    {
        JsonDataSource dataSource;
        HeapLDSCatalog catalog;
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

    TEST_CASE("register object with array property containing pod items with max items metadata truncates the array")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("find object record pod property value defined in object")
    {
        HeapLDSCatalog catalog;
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
    
    TEST_CASE("find object record pod property value defined in base object")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("find object record pod property default value defined in base type")
    {
        HeapLDSCatalog catalog;
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

    TEST_CASE("find object record pod property value that is not defined in object or base object or base type returns null")
    {
        HeapLDSCatalog catalog;
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
}