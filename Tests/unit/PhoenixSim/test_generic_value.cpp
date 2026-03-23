#include <doctest/doctest.h>
#include <PhoenixSim/Reflection/GenericValue.h>
#include <PhoenixSim/FixedPoint/FixedTransform.h>   // Vec2, Transform2D (PHX_REGISTER_EXTERNAL_TYPE)
#include <PhoenixSim/FixedPoint/FixedTypes.h>         // Distance, Angle (PHX_REGISTER_EXTERNAL_TYPE)

using namespace Phoenix;

TEST_SUITE("GenericValue")
{
    TEST_CASE("bool roundtrip")
    {
        CHECK(GenericValue::Borrow(true).As<bool>()  == true);
        CHECK(GenericValue::Borrow(false).As<bool>() == false);
    }

    TEST_CASE("integer roundtrips")
    {
        CHECK(GenericValue::Borrow((int8_t)  -5).As<int8_t>()   == -5);
        CHECK(GenericValue::Borrow((uint8_t)  5).As<uint8_t>()  ==  5);
        CHECK(GenericValue::Borrow((int16_t)-500).As<int16_t>() == -500);
        CHECK(GenericValue::Borrow((uint16_t)500).As<uint16_t>()== 500);
        CHECK(GenericValue::Borrow((int32_t)-1).As<int32_t>()   == -1);
        CHECK(GenericValue::Borrow((uint32_t)42).As<uint32_t>() == 42);
        CHECK(GenericValue::Borrow((int64_t)-1LL).As<int64_t>() == -1LL);
        CHECK(GenericValue::Borrow((uint64_t)99).As<uint64_t>() == 99ULL);
    }

    TEST_CASE("float roundtrip")
    {
        CHECK(GenericValue::Borrow(3.14f).As<float>()   == doctest::Approx(3.14f));
        CHECK(GenericValue::Borrow(2.718).As<double>()  == doctest::Approx(2.718));
    }

    TEST_CASE("string roundtrip")
    {
        CHECK(GenericValue::Borrow(std::string("hello")).As<std::string>() == "hello");
    }

    TEST_CASE("FName roundtrip")
    {
        FName name("TestName");
        CHECK(GenericValue::Borrow(name).As<FName>() == name);
    }

    TEST_CASE("Void returns unknown primitive type")
    {
        GenericValue v = GenericValue::Void();
        CHECK(v.Type.IsVoid());
    }

    // ── String copy / move ────────────────────────────────────────────────────
    // These exercise the explicit copy/move constructors and assignment operators
    // added to handle the std::string member of the internal union correctly.

    TEST_CASE("string copy constructor")
    {
        GenericValue a = GenericValue::Borrow(std::string("hello"));
        GenericValue b = a;
        CHECK(b.As<std::string>() == "hello");
        CHECK(a.As<std::string>() == "hello");  // original intact
    }

    TEST_CASE("string move constructor")
    {
        GenericValue a = GenericValue::Borrow(std::string("world"));
        GenericValue b = std::move(a);
        CHECK(b.As<std::string>() == "world");
    }

    TEST_CASE("string copy assignment")
    {
        GenericValue a = GenericValue::Borrow(std::string("copy"));
        GenericValue b;
        b = a;
        CHECK(b.As<std::string>() == "copy");
        CHECK(a.As<std::string>() == "copy");
    }

    TEST_CASE("string move assignment")
    {
        GenericValue a = GenericValue::Borrow(std::string("move"));
        GenericValue b;
        b = std::move(a);
        CHECK(b.As<std::string>() == "move");
    }

    // ── Registered struct type ────────────────────────────────────────────────

    TEST_CASE("struct roundtrip — IsStruct and Descriptor set")
    {
        Vec2 v;
        v.X = Distance(1.5f);
        v.Y = Distance(2.5f);
        GenericValue gv = GenericValue::Borrow(v);
        CHECK(gv.Type.IsStruct());
        REQUIRE(gv.Type.Descriptor != nullptr);
        CHECK(std::string(gv.Type.Descriptor->GetCName()) == "Vec2");
        Vec2 out = gv.As<Vec2>();
        CHECK(out.X == v.X);
        CHECK(out.Y == v.Y);
    }

    // ── FixedPoint type ───────────────────────────────────────────────────────
    // FixedPoint values carry a TypeDescriptor (for FractionalBits) but are
    // NOT structs — IsStruct() must stay false even when Descriptor is set.

    TEST_CASE("FixedPoint roundtrip")
    {
        Distance d(3.5f);
        GenericValue gv = GenericValue::Borrow(d);
        CHECK(gv.Type.Primitive == EGenericValueType::FixedPoint);
        CHECK(gv.Type.IsPrimitive());
        CHECK(!gv.Type.IsStruct());
        CHECK(gv.As<Distance>() == d);
    }

    TEST_CASE("FixedPoint GenericValue carries TypeDescriptor")
    {
        GenericValue gv = GenericValue::Borrow(Distance(1.0f));
        REQUIRE(gv.Type.Descriptor != nullptr);
        CHECK(std::string(gv.Type.Descriptor->GetCName()) == "Distance");
    }

    TEST_CASE("FixedPoint TypeDescriptor has FractionalBits metadata")
    {
        GenericValue gv = GenericValue::Borrow(Distance(1.0f));
        REQUIRE(gv.Type.Descriptor != nullptr);
        const auto& meta = gv.Type.Descriptor->GetMetadata();
        auto it = meta.find("FractionalBits");
        REQUIRE(it != meta.end());
        CHECK(it->second == "12");  // Distance = TFixed<12>
    }

    TEST_CASE("Angle TypeDescriptor has correct FractionalBits")
    {
        GenericValue gv = GenericValue::Borrow(Angle(1.0f));
        REQUIRE(gv.Type.Descriptor != nullptr);
        const auto& meta = gv.Type.Descriptor->GetMetadata();
        CHECK(meta.at("FractionalBits") == "20");  // Angle = TFixed<20>
    }
}
