
#include <doctest/doctest.h>

#include "Phoenix/FixedPoint/FixedVector.h"

using namespace Phoenix;

TEST_SUITE("TVec2")
{
    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    TEST_CASE("default-constructed Vec2 is zero")
    {
        Vec2 v;
        CHECK((float)v.X == doctest::Approx(0.0f));
        CHECK((float)v.Y == doctest::Approx(0.0f));
    }

    TEST_CASE("Vec2(x, y) stores components")
    {
        Vec2 v(3.0f, -4.0f);
        CHECK((float)v.X == doctest::Approx( 3.0f).epsilon(0.01f));
        CHECK((float)v.Y == doctest::Approx(-4.0f).epsilon(0.01f));
    }

    TEST_CASE("Vec2(scalar) sets both components")
    {
        Vec2 v(Distance(5.0));
        CHECK((float)v.X == doctest::Approx(5.0f).epsilon(0.01f));
        CHECK((float)v.Y == doctest::Approx(5.0f).epsilon(0.01f));
    }

    // -------------------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------------------

    TEST_CASE("Zero constant is (0, 0)")
    {
        CHECK(Vec2::Zero.X == Distance(0));
        CHECK(Vec2::Zero.Y == Distance(0));
    }

    TEST_CASE("One constant is (1, 1)")
    {
        CHECK(Vec2::One.X == Distance(1));
        CHECK(Vec2::One.Y == Distance(1));
    }

    TEST_CASE("XAxis is (1, 0)")
    {
        CHECK(Vec2::XAxis.X == Distance(1));
        CHECK(Vec2::XAxis.Y == Distance(0));
    }

    TEST_CASE("YAxis is (0, 1)")
    {
        CHECK(Vec2::YAxis.X == Distance(0));
        CHECK(Vec2::YAxis.Y == Distance(1));
    }

    // -------------------------------------------------------------------------
    // Arithmetic
    // -------------------------------------------------------------------------

    TEST_CASE("vector addition")
    {
        Vec2 a(1.0f, 2.0f), b(3.0f, 4.0f);
        Vec2 r = a + b;
        CHECK((float)r.X == doctest::Approx(4.0f).epsilon(0.01f));
        CHECK((float)r.Y == doctest::Approx(6.0f).epsilon(0.01f));
    }

    TEST_CASE("vector subtraction")
    {
        Vec2 a(5.0f, 7.0f), b(2.0f, 3.0f);
        Vec2 r = a - b;
        CHECK((float)r.X == doctest::Approx(3.0f).epsilon(0.01f));
        CHECK((float)r.Y == doctest::Approx(4.0f).epsilon(0.01f));
    }

    TEST_CASE("vector * scalar")
    {
        Vec2 v(2.0f, 3.0f);
        Vec2 r = v * Distance(2.0);
        CHECK((float)r.X == doctest::Approx(4.0f).epsilon(0.05f));
        CHECK((float)r.Y == doctest::Approx(6.0f).epsilon(0.05f));
    }

    TEST_CASE("vector compound += and -=")
    {
        Vec2 a(1.0f, 2.0f);
        a += Vec2(3.0f, 4.0f);
        CHECK((float)a.X == doctest::Approx(4.0f).epsilon(0.01f));
        CHECK((float)a.Y == doctest::Approx(6.0f).epsilon(0.01f));
        a -= Vec2(1.0f, 1.0f);
        CHECK((float)a.X == doctest::Approx(3.0f).epsilon(0.01f));
        CHECK((float)a.Y == doctest::Approx(5.0f).epsilon(0.01f));
    }

    // -------------------------------------------------------------------------
    // Geometric operations
    // -------------------------------------------------------------------------

    TEST_CASE("Cross product of XAxis and YAxis is 1")
    {
        auto c = Vec2::Cross(Vec2::XAxis, Vec2::YAxis);
        CHECK((double)c == doctest::Approx(1.0).epsilon(0.01));
    }

    TEST_CASE("Cross product is anti-commutative")
    {
        auto cxy = Vec2::Cross(Vec2::XAxis, Vec2::YAxis);
        auto cyx = Vec2::Cross(Vec2::YAxis, Vec2::XAxis);
        CHECK((double)cxy == doctest::Approx(-(double)cyx).epsilon(0.01));
    }

    TEST_CASE("Cross product of parallel vectors is zero")
    {
        Vec2 a(2.0f, 0.0f), b(5.0f, 0.0f);
        auto c = Vec2::Cross(a, b);
        CHECK((double)c == doctest::Approx(0.0).epsilon(0.01));
    }

    TEST_CASE("SqrxQ gives squared distance in raw Q units")
    {
        // Vec2(3, 4): squared magnitude = 3^2 + 4^2 = 25 in real units
        // In Q units: (3*D)^2 + (4*D)^2 = 25 * D^2, but SqrxQ(v,v) = Sqrx(X.Value,X.Value)+...
        Vec2 v(3.0f, 4.0f);
        int64 sq = Vec2::SqrxQ(v);
        // sq = 3.Value^2 + 4.Value^2 = (3*4096)^2 + (4*4096)^2 = 9*D^2 + 16*D^2 = 25*D^2
        constexpr int64 D = Distance::D;
        CHECK(sq == 25 * D * D);
    }

    TEST_CASE("Perpendicular clockwise rotates (1,0) to (0,-1)")
    {
        Vec2 cw = Vec2::Perpendicular(Vec2::XAxis, Vec2::EPerpendicularDir::Clockwise);
        CHECK((float)cw.X == doctest::Approx(0.0f).epsilon(0.001f));
        CHECK((float)cw.Y == doctest::Approx(-1.0f).epsilon(0.01f));
    }

    TEST_CASE("Perpendicular counter-clockwise rotates (1,0) to (0,1)")
    {
        Vec2 ccw = Vec2::Perpendicular(Vec2::XAxis, Vec2::EPerpendicularDir::CounterClockwise);
        CHECK((float)ccw.X == doctest::Approx(0.0f).epsilon(0.001f));
        CHECK((float)ccw.Y == doctest::Approx(1.0f).epsilon(0.01f));
    }

    TEST_CASE("Perpendicular of zero vector is zero vector")
    {
        Vec2 z = Vec2::Perpendicular(Vec2::Zero);
        CHECK((float)z.X == doctest::Approx(0.0f));
        CHECK((float)z.Y == doctest::Approx(0.0f));
    }

    TEST_CASE("Midpoint is average of two points")
    {
        Vec2 a(0.0f, 0.0f), b(4.0f, 6.0f);
        Vec2 m = Vec2::Midpoint(a, b);
        CHECK((float)m.X == doctest::Approx(2.0f).epsilon(0.01f));
        CHECK((float)m.Y == doctest::Approx(3.0f).epsilon(0.01f));
    }

    TEST_CASE("Equals returns true within threshold")
    {
        Vec2 a(1.0f, 1.0f);
        Vec2 b(1.0f, 1.0f);
        Vec2 c(2.0f, 2.0f);
        CHECK( Vec2::Equals(a, b));
        CHECK(!Vec2::Equals(a, c));
    }
}
