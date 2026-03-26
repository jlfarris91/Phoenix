#include <doctest/doctest.h>
#include <PhoenixSim/Reflection/Reflection.h>
#include <PhoenixSim/Reflection/Registration.h>
#include <PhoenixSim/Reflection/TypeRegistry.h>
#include <PhoenixSim/FixedPoint/FixedTransform.h>   // Vec2 (external type)
#include <PhoenixSim/FixedPoint/FixedTypes.h>         // Distance

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
    bool  CanNormalize() const { return (X*X + Y*Y) > 0.0f; }
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
        .Method("Normalize", &ReflPoint::Normalize, &ReflPoint::CanNormalize);
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
        const TypeDescriptor* desc = TypeRegistry::Get(ReflPoint::StaticTypeName);
        REQUIRE(desc != nullptr);
    }

    TEST_CASE("Get returns nullptr for unknown type")
    {
        CHECK(TypeRegistry::Get(FName("NoSuchType")) == nullptr);
    }

    TEST_CASE("GetOrCreate is idempotent — returns same descriptor each call")
    {
        const TypeDescriptor& a = TypeRegistry::GetOrCreate<ReflPoint>();
        const TypeDescriptor& b = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(&a == &b);
    }

    TEST_CASE("CName returns unqualified type name")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(std::string(desc.GetCName()) == "ReflPoint");
    }

    TEST_CASE("GetQualifiedCName returns fully qualified name for namespaced type")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflTestNs::ReflNamed>();
        CHECK(std::string(desc.GetQualifiedCName()) == "ReflTestNs::ReflNamed");
    }

    TEST_CASE("GetQualifiedCName falls back to CName for file-scope type")
    {
        // ReflPoint is at file scope — qualified name has no namespace prefix.
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(std::string(desc.GetQualifiedCName()) == "ReflPoint");
    }
}

// ── Properties ────────────────────────────────────────────────────────────────

TEST_SUITE("PropertyDescriptor")
{
    TEST_CASE("registered fields appear in GetProperties")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(desc.GetProperties().count("X") == 1);
        CHECK(desc.GetProperties().count("Y") == 1);
    }

    TEST_CASE("field accessor reads correct value")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        ReflPoint p; p.X = 3.0f; p.Y = 4.0f;
        const auto& xProp = desc.GetProperties().at("X");
        CHECK(xProp.PropertyAccessor->Get<float>(&p) == doctest::Approx(3.0f));
    }

    TEST_CASE("field accessor writes correct value")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        ReflPoint p; p.X = 0.0f;
        const auto& xProp = desc.GetProperties().at("X");
        xProp.PropertyAccessor->Set(&p, 7.5f);
        CHECK(p.X == doctest::Approx(7.5f));
    }

    TEST_CASE("field accessor ValueType is Float")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(desc.GetProperties().at("X").ValueType == EGenericValueType::Float);
    }
}

// ── Methods ───────────────────────────────────────────────────────────────────

TEST_SUITE("MethodDescriptor")
{
    TEST_CASE("registered method appears in GetMethods")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(desc.GetMethods().count("Normalize") == 1);
    }

    TEST_CASE("instance method IsStatic returns false")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        CHECK(desc.GetMethods().at("Normalize").IsStatic() == false);
    }

    TEST_CASE("CanExecute with predicate returning true")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        ReflPoint p; p.X = 1.0f; p.Y = 0.0f;
        CHECK(desc.GetMethods().at("Normalize").CanExecute(&p) == true);
    }

    TEST_CASE("Execute invokes method and mutates object")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
        ReflPoint p; p.X = 3.0f; p.Y = 4.0f;  // length == 5
        desc.GetMethods().at("Normalize").Execute(&p);
        CHECK(p.X == doctest::Approx(0.6f));
        CHECK(p.Y == doctest::Approx(0.8f));
    }
}

// ── MakeMethodDescriptor (free-standing) ─────────────────────────────────────

TEST_SUITE("MakeMethodDescriptor")
{
    static float Multiply(float a, float b) { return a * b; }

    TEST_CASE("name is set correctly")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        CHECK(d.Name == "Multiply");
    }

    TEST_CASE("free function is static")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        CHECK(d.IsStatic() == true);
    }

    TEST_CASE("param count matches signature")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        CHECK(d.Params.size() == 2);
    }

    TEST_CASE("return type is Float")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        CHECK(d.Return.Type.Primitive == EGenericValueType::Float);
    }

    TEST_CASE("Execute returns correct result")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        GenericValue args[2] = { GenericValue::Borrow(3.0f), GenericValue::Borrow(4.0f) };
        CHECK(d.Execute(nullptr, args).As<float>() == doctest::Approx(12.0f));
    }

    TEST_CASE("CanExecute is true when no predicate set")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        CHECK(d.CanExecute(nullptr) == true);
    }

    TEST_CASE("CanExecute respects predicate")
    {
        auto d = MakeMethodDescriptor("Multiply", Multiply);
        d.CanExecutePredicate = [](void*) { return false; };
        CHECK(d.CanExecute(nullptr) == false);
    }
}

// ── Inheritance ───────────────────────────────────────────────────────────────

TEST_SUITE("TypeDescriptor inheritance")
{
    TEST_CASE("derived type has own field")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflDerived>();
        CHECK(desc.GetProperties().count("DerivedValue") == 1);
    }

    TEST_CASE("IsA returns true for registered base")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflDerived>();
        CHECK(desc.IsA<ReflBase>() == true);
    }

    TEST_CASE("IsA returns false for unrelated type")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflDerived>();
        CHECK(desc.IsA<ReflPoint>() == false);
    }
}

// ── DefaultConstruct / Destruct ───────────────────────────────────────────────

TEST_SUITE("TypeDescriptor DefaultConstruct")
{
    TEST_CASE("DefaultConstruct zero-initializes a registered type")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflPoint>();
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
        CHECK(std::string(desc->GetCName()) == "Vec2");
    }

    TEST_CASE("externally-registered type has its fields")
    {
        const TypeDescriptor* desc = TypeRegistry::Get(FName("Vec2"));
        REQUIRE(desc != nullptr);
        CHECK(desc->GetProperties().count("X") == 1);
        CHECK(desc->GetProperties().count("Y") == 1);
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
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflWithNested>();
        CHECK(desc.GetProperties().at("Origin").ValueType == EGenericValueType::Struct);
    }

    TEST_CASE("StructDescriptor points at the nested type's descriptor")
    {
        const TypeDescriptor& desc    = TypeRegistry::GetOrCreate<ReflWithNested>();
        const TypeDescriptor& nested  = TypeRegistry::GetOrCreate<ReflPoint>();
        const auto& prop = desc.GetProperties().at("Origin");
        REQUIRE(prop.StructDescriptor != nullptr);
        CHECK(prop.StructDescriptor == &nested);
    }

    TEST_CASE("struct-valued field accessor reads nested value")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<ReflWithNested>();
        ReflWithNested obj;
        obj.Origin.X = 7.0f;
        obj.Origin.Y = 8.0f;
        ReflPoint out = desc.GetProperties().at("Origin").PropertyAccessor->Get<ReflPoint>(&obj);
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
        .Field("Speed", &ReflNumeric::Speed, [](PropertyDescriptorBuilder<float>& b) {
            b.MinValue(0.0f).MaxValue(100.0f).Step(0.5f);
        })
        .Field("Count", &ReflNumeric::Count, [](PropertyDescriptorBuilder<int32_t>& b) {
            b.MinValue(0).MaxValue(99);
        });
    return true;
}();

TEST_SUITE("PropertyDescriptorBuilder numeric metadata")
{
    TEST_CASE("MinValue and MaxValue written to Metadata for float field")
    {
        const auto& prop = TypeRegistry::GetOrCreate<ReflNumeric>().GetProperties().at("Speed");
        REQUIRE(prop.Metadata.count("MinValue") == 1);
        REQUIRE(prop.Metadata.count("MaxValue") == 1);
        CHECK(std::stof(prop.Metadata.at("MinValue")) == doctest::Approx(0.0f));
        CHECK(std::stof(prop.Metadata.at("MaxValue")) == doctest::Approx(100.0f));
    }

    TEST_CASE("Step written to Metadata for float field")
    {
        const auto& prop = TypeRegistry::GetOrCreate<ReflNumeric>().GetProperties().at("Speed");
        REQUIRE(prop.Metadata.count("Step") == 1);
        CHECK(std::stof(prop.Metadata.at("Step")) == doctest::Approx(0.5f));
    }

    TEST_CASE("MinValue and MaxValue written to Metadata for int field")
    {
        const auto& prop = TypeRegistry::GetOrCreate<ReflNumeric>().GetProperties().at("Count");
        REQUIRE(prop.Metadata.count("MinValue") == 1);
        REQUIRE(prop.Metadata.count("MaxValue") == 1);
        CHECK(std::stoi(prop.Metadata.at("MinValue")) == 0);
        CHECK(std::stoi(prop.Metadata.at("MaxValue")) == 99);
    }
}

struct ReflDistanceHolder { Distance D; PHX_DECLARE_TYPE(ReflDistanceHolder) };
static bool s_ReflDistanceHolderRegistered = []() {
    TypeDescriptorBuilder<ReflDistanceHolder>().Field("D", &ReflDistanceHolder::D);
    return true;
}();

// ── TypeDescriptor metadata (TFixed domain types) ─────────────────────────────
//
// TypeDescriptorMetadataProvider<TFixed<Tb,T>> auto-populates FractionalBits
// in the TypeDescriptor when GetOrCreate is first called.

TEST_SUITE("TypeDescriptor metadata")
{
    TEST_CASE("Distance TypeDescriptor has FractionalBits == 12")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<Distance>();
        auto it = desc.GetMetadata().find("FractionalBits");
        REQUIRE(it != desc.GetMetadata().end());
        CHECK(it->second == "12");
    }

    TEST_CASE("Angle TypeDescriptor has FractionalBits == 20")
    {
        const TypeDescriptor& desc = TypeRegistry::GetOrCreate<Angle>();
        auto it = desc.GetMetadata().find("FractionalBits");
        REQUIRE(it != desc.GetMetadata().end());
        CHECK(it->second == "20");
    }

    TEST_CASE("Distance PropertyDescriptor also has FractionalBits in its Metadata")
    {
        // A field of type Distance should have FractionalBits in PropertyDescriptor::Metadata
        // via TypeDescriptorMetadataProvider (same provider, property-level copy).
        // ReflDistanceHolder is defined at namespace scope (below) due to MSVC's restriction
        // on static constexpr members in locally-defined classes.
        const auto& prop = TypeRegistry::GetOrCreate<ReflDistanceHolder>().GetProperties().at("D");
        REQUIRE(prop.Metadata.count("FractionalBits") == 1);
        CHECK(prop.Metadata.at("FractionalBits") == "12");
    }
}
