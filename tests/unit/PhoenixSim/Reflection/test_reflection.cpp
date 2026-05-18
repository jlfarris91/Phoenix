#include <doctest/doctest.h>
#include <cmath>
#include <Phoenix/Reflection/Registration.h>
#include <Phoenix/Reflection/TypeRegistry.h>
#include <Phoenix.Sim/FixedPoint/FixedTransform.h>   // Vec2 (external type)
#include <Phoenix.Sim/FixedPoint/FixedTypes.h>         // Distance

using namespace Phoenix;

// ── Test types ────────────────────────────────────────────────────────────────
//
// Using PHX_DECLARE_TYPE so registration stays inline (no separate .cpp needed).
// Each type has a unique name to avoid cross-test descriptor pollution.

struct ReflPoint
{
    float X = 0.0f;
    float Y = 0.0f;
    float LengthSq() const { return X * X + Y * Y; }
    void  Normalize() { float l = std::sqrt(X*X + Y*Y); if (l > 0) { X /= l; Y /= l; } }
    PHX_DECLARE_TYPE(ReflPoint)
};

struct ReflBase
{
    int BaseValue = 0;
    PHX_DECLARE_TYPE(ReflBase)
};

struct ReflDerived : ReflBase
{
    int DerivedValue = 0;
    PHX_DECLARE_TYPE(ReflDerived, ReflBase)
};

// Register fields/methods once at static-init time
static bool s_ReflPointRegistered = []() {
    TypeDescriptorBuilder<ReflPoint>()
        .Field("X", &ReflPoint::X)
        .Field("Y", &ReflPoint::Y)
        .Method("Normalize", &ReflPoint::Normalize);
    return true;
}();

static bool s_ReflBaseRegistered = []() {
    TypeDescriptorBuilder<ReflBase>()
        .Field("BaseValue", &ReflBase::BaseValue);
    return true;
}();

static bool s_ReflDerivedRegistered = []() {
    TypeDescriptorBuilder<ReflDerived>()
        .Base<ReflBase>()
        .Field("DerivedValue", &ReflDerived::DerivedValue);
    return true;
}();

// Type inside a namespace — used to verify qualified name extraction.
namespace ReflTestNs
{
    struct ReflNamed
    {
        int Value = 0;
        PHX_DECLARE_TYPE(ReflNamed)
    };
}

// ── TypeRegistry ──────────────────────────────────────────────────────────────

TEST_SUITE("TypeRegistry")
{
    TEST_CASE("Get returns descriptor for registered type")
    {
        const TypeDescriptor* desc = TypeRegistry::Get(StaticTypeName<ReflPoint>::TypeId);
        REQUIRE(desc != nullptr);
    }

    TEST_CASE("Get returns nullptr for unknown type")
    {
        CHECK(TypeRegistry::Get(FName("NoSuchType")) == nullptr);
    }

    TEST_CASE("Get is idempotent — returns same descriptor each call")
    {
        const TypeDescriptor& a = TypeRegistry::Get<ReflPoint>();
        const TypeDescriptor& b = TypeRegistry::Get<ReflPoint>();
        CHECK(&a == &b);
    }

    TEST_CASE("CName returns unqualified type name")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(std::string(desc.GetName()) == "ReflPoint");
    }

    TEST_CASE("GetQualifiedCName returns fully qualified name for namespaced type")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflTestNs::ReflNamed>();
        CHECK(std::string(desc.GetQualifiedName()) == "ReflTestNs::ReflNamed");
    }

    TEST_CASE("GetQualifiedCName falls back to CName for file-scope type")
    {
        // ReflPoint is at file scope — qualified name has no namespace prefix.
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(std::string(desc.GetQualifiedName()) == "ReflPoint");
    }
}

// ── Fields ────────────────────────────────────────────────────────────────

TEST_SUITE("FieldDescriptor")
{
    TEST_CASE("registered fields appear in GetFields")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(desc.GetFields().count("X") == 1);
        CHECK(desc.GetFields().count("Y") == 1);
    }

    TEST_CASE("field accessor reads correct value")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        ReflPoint p; p.X = 3.0f; p.Y = 4.0f;
        const auto& xProp = desc.GetFields().at("X");
        CHECK(xProp.Get<float>(&p) == doctest::Approx(3.0f));
    }

    TEST_CASE("field accessor writes correct value")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        ReflPoint p; p.X = 0.0f;
        const auto& xProp = desc.GetFields().at("X");
        xProp.Set(&p, 7.5f);
        CHECK(p.X == doctest::Approx(7.5f));
    }

    TEST_CASE("field accessor ValueType is Float")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(desc.GetFields().at("X").GetType()->GetTypeId() == StaticTypeName<float>::TypeId);
    }
}

// ── Methods ───────────────────────────────────────────────────────────────────

TEST_SUITE("MethodDescriptor")
{
    TEST_CASE("registered method appears in GetMethods")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(desc.GetMethods().count("Normalize") == 1);
    }

    TEST_CASE("instance method IsStatic returns false")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        CHECK(desc.GetMethods().at("Normalize").IsStatic() == false);
    }

    TEST_CASE("CanExecute with predicate returning true")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        ReflPoint p; p.X = 1.0f; p.Y = 0.0f;
        CHECK(desc.GetMethods().at("Normalize").CanExecute(&p) == true);
    }

    TEST_CASE("Execute invokes method and mutates object")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        ReflPoint p; p.X = 3.0f; p.Y = 4.0f;  // length == 5
        desc.GetMethods().at("Normalize").Execute(&p);
        CHECK(p.X == doctest::Approx(0.6f));
        CHECK(p.Y == doctest::Approx(0.8f));
    }
}

// ── Inheritance ───────────────────────────────────────────────────────────────

TEST_SUITE("TypeDescriptor inheritance")
{
    TEST_CASE("derived type has own field")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflDerived>();
        CHECK(desc.GetFields().count("DerivedValue") == 1);
    }

    TEST_CASE("IsA returns true for registered base")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflDerived>();
        CHECK(desc.IsA<ReflBase>() == true);
    }

    TEST_CASE("IsA returns false for unrelated type")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflDerived>();
        CHECK(desc.IsA<ReflPoint>() == false);
    }
}

// ── DefaultConstruct / Destruct ───────────────────────────────────────────────

TEST_SUITE("TypeDescriptor DefaultConstruct")
{
    TEST_CASE("DefaultConstruct zero-initializes a registered type")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflPoint>();
        alignas(ReflPoint) std::byte storage[sizeof(ReflPoint)];
        // Fill with garbage first
        std::memset(storage, 0xFF, sizeof(storage));
        desc.DefaultConstruct(storage);
        auto* p = std::launder(reinterpret_cast<ReflPoint*>(storage));
        CHECK(p->X == doctest::Approx(0.0f));
        CHECK(p->Y == doctest::Approx(0.0f));
        desc.Destruct(storage);
    }
}

// ── External type registration ────────────────────────────────────────────────
//
// Vec2 is registered via PHX_REGISTER_EXTERNAL_TYPE (no in-class PHX_DECLARE_TYPE).
// Its fields are registered in FixedTypeRegistrations.cpp.

TEST_SUITE("External type registration")
{
    TEST_CASE("TypeRegistry::Get finds externally-registered type by name")
    {
        const TypeDescriptor* desc = TypeRegistry::Get(FName("Vec2"));
        REQUIRE(desc != nullptr);
        CHECK(std::string(desc->GetAliasOrName()) == "Vec2");
    }

    TEST_CASE("externally-registered type has its fields")
    {
        const TypeDescriptor* desc = TypeRegistry::Get(FName("Vec2"));
        REQUIRE(desc != nullptr);
        CHECK(desc->GetFields().count("X") == 1);
        CHECK(desc->GetFields().count("Y") == 1);
    }
}

// ── Struct-valued properties ──────────────────────────────────────────────────
//
// When a field's type is itself a registered struct, PropertyDescriptor should
// have ValueType==Struct and StructDescriptor pointing at that type's descriptor.

struct ReflWithNested
{
    ReflPoint Origin;
    PHX_DECLARE_TYPE(ReflWithNested)
};

static bool s_ReflWithNestedRegistered = []() {
    TypeDescriptorBuilder<ReflWithNested>()
        .Field("Origin", &ReflWithNested::Origin);
    return true;
}();

TEST_SUITE("PropertyDescriptor struct-valued field")
{
    TEST_CASE("ValueType is Struct")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflWithNested>();
        CHECK(desc.GetFields().at("Origin").GetType()->GetTypeId() == StaticTypeName<ReflPoint>::TypeId);
    }

    TEST_CASE("StructDescriptor points at the nested type's descriptor")
    {
        const TypeDescriptor& desc    = TypeRegistry::Get<ReflWithNested>();
        const TypeDescriptor& nested  = TypeRegistry::Get<ReflPoint>();
        const auto& prop = desc.GetFields().at("Origin");
        REQUIRE(prop.GetType() != nullptr);
        CHECK(prop.GetType() == &nested);
    }

    TEST_CASE("struct-valued field accessor reads nested value")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<ReflWithNested>();
        ReflWithNested obj;
        obj.Origin.X = 7.0f;
        obj.Origin.Y = 8.0f;
        ReflPoint out = desc.GetFields().at("Origin").Get<ReflPoint>(&obj);
        CHECK(out.X == doctest::Approx(7.0f));
        CHECK(out.Y == doctest::Approx(8.0f));
    }
}

// ── PropertyDescriptorBuilder numeric metadata ────────────────────────────────
//
// Numeric fields registered with the optional builder callback should store
// MinValue / MaxValue / Step in PropertyDescriptor::Metadata.

struct ReflNumeric
{
    float Speed = 0.0f;
    int32_t Count = 0;
    PHX_DECLARE_TYPE(ReflNumeric)
};

static bool s_ReflNumericRegistered = []() {
    TypeDescriptorBuilder<ReflNumeric>()
        .Field("Speed", &ReflNumeric::Speed, [](FieldDescriptorBuilder<float>& b) {
            b.MinValue(0.0f).MaxValue(100.0f).Step(0.5f);
        })
        .Field("Count", &ReflNumeric::Count, [](FieldDescriptorBuilder<int32_t>& b) {
            b.MinValue(0).MaxValue(99);
        });
    return true;
}();

TEST_SUITE("PropertyDescriptorBuilder numeric metadata")
{
    TEST_CASE("MinValue and MaxValue written to Metadata for float field")
    {
        const auto& prop = TypeRegistry::Get<ReflNumeric>().GetFields().at("Speed");
        REQUIRE(prop.GetMetadata().count("MinValue") == 1);
        REQUIRE(prop.GetMetadata().count("MaxValue") == 1);
        CHECK(std::stof(prop.GetMetadata().at("MinValue")) == doctest::Approx(0.0f));
        CHECK(std::stof(prop.GetMetadata().at("MaxValue")) == doctest::Approx(100.0f));
    }

    TEST_CASE("Step written to Metadata for float field")
    {
        const auto& prop = TypeRegistry::Get<ReflNumeric>().GetFields().at("Speed");
        REQUIRE(prop.GetMetadata().count("Step") == 1);
        CHECK(std::stof(prop.GetMetadata().at("Step")) == doctest::Approx(0.5f));
    }

    TEST_CASE("MinValue and MaxValue written to Metadata for int field")
    {
        const auto& prop = TypeRegistry::Get<ReflNumeric>().GetFields().at("Count");
        REQUIRE(prop.GetMetadata().count("MinValue") == 1);
        REQUIRE(prop.GetMetadata().count("MaxValue") == 1);
        CHECK(std::stoi(prop.GetMetadata().at("MinValue")) == 0);
        CHECK(std::stoi(prop.GetMetadata().at("MaxValue")) == 99);
    }
}

struct ReflDistanceHolder { Phoenix::Distance D; PHX_DECLARE_TYPE(ReflDistanceHolder) };
static bool s_ReflDistanceHolderRegistered = []() {
    TypeDescriptorBuilder<ReflDistanceHolder>().Field("D", &ReflDistanceHolder::D);
    return true;
}();

// ── TypeDescriptor metadata (TFixed domain types) ─────────────────────────────
//
// TypeDescriptorMetadataProvider<TFixed<Tb,T>> auto-populates FractionalBits
// in the TypeDescriptor when Get is first called.

TEST_SUITE("TypeDescriptor metadata")
{
    TEST_CASE("Distance TypeDescriptor has FractionalBits == 12")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<Phoenix::Distance>();
        auto it = desc.GetMetadata().find("FractionalBits");
        REQUIRE(it != desc.GetMetadata().end());
        CHECK(it->second == "12");
    }

    TEST_CASE("Angle TypeDescriptor has FractionalBits == 20")
    {
        const TypeDescriptor& desc = TypeRegistry::Get<Phoenix::Angle>();
        auto it = desc.GetMetadata().find("FractionalBits");
        REQUIRE(it != desc.GetMetadata().end());
        CHECK(it->second == "20");
    }
}
