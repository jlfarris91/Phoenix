
#include <doctest/doctest.h>

#include "PhoenixSim/FixedPoint/FixedMath.h"

using namespace Phoenix;

TEST_SUITE("FixedPoint")
{
    // -------------------------------------------------------------------------
    // Construction and conversion
    // -------------------------------------------------------------------------

    TEST_CASE("default-constructed TFixed is zero")
    {
        Distance d;
        CHECK((int32)d == 0);
    }

    TEST_CASE("TFixed constructed from int round-trips")
    {
        for (int32 v : { -100, -1, 0, 1, 42, 100 })
        {
            Distance d(v);
            CHECK((int32)d == v);
        }
    }

    TEST_CASE("TFixed constructed from float round-trips within epsilon")
    {
        for (float v : { -3.5f, -1.0f, 0.0f, 0.5f, 1.25f, 7.75f })
        {
            Distance d(v);
            CHECK((float)d == doctest::Approx(v).epsilon(0.001f));
        }
    }

    TEST_CASE("TFixed constructed from double round-trips within epsilon")
    {
        for (double v : { -4.0, -0.25, 0.0, 1.0, 3.75, 10.5 })
        {
            Distance d(v);
            CHECK((double)d == doctest::Approx(v).epsilon(0.0005));
        }
    }

    // -------------------------------------------------------------------------
    // Arithmetic
    // -------------------------------------------------------------------------

    TEST_CASE("addition")
    {
        CHECK((double)(Distance(3.0) + Distance(4.0))  == doctest::Approx(7.0).epsilon(0.001));
        CHECK((double)(Distance(-2.0) + Distance(5.0)) == doctest::Approx(3.0).epsilon(0.001));
    }

    TEST_CASE("subtraction")
    {
        CHECK((double)(Distance(10.0) - Distance(3.0))  == doctest::Approx(7.0).epsilon(0.001));
        CHECK((double)(Distance(1.0) - Distance(4.0))   == doctest::Approx(-3.0).epsilon(0.001));
    }

    TEST_CASE("multiplication")
    {
        CHECK((double)(Distance(3.0) * Distance(4.0))   == doctest::Approx(12.0).epsilon(0.01));
        CHECK((double)(Distance(-2.0) * Distance(3.0))  == doctest::Approx(-6.0).epsilon(0.01));
        CHECK((double)(Distance(0.5) * Distance(8.0))   == doctest::Approx(4.0).epsilon(0.01));
    }

    TEST_CASE("division")
    {
        CHECK((double)(Distance(10.0) / Distance(2.0))  == doctest::Approx(5.0).epsilon(0.001));
        CHECK((double)(Distance(7.0) / Distance(2.0))   == doctest::Approx(3.5).epsilon(0.001));
        CHECK((double)(Distance(-9.0) / Distance(3.0))  == doctest::Approx(-3.0).epsilon(0.001));
    }

    TEST_CASE("compound assignment operators")
    {
        Distance a(10.0);
        a += Distance(3.0);
        CHECK((double)a == doctest::Approx(13.0).epsilon(0.001));
        a -= Distance(5.0);
        CHECK((double)a == doctest::Approx(8.0).epsilon(0.001));
        a *= Distance(2.0);
        CHECK((double)a == doctest::Approx(16.0).epsilon(0.01));
        a /= Distance(4.0);
        CHECK((double)a == doctest::Approx(4.0).epsilon(0.001));
    }

    TEST_CASE("unary negation")
    {
        CHECK((double)(-Distance(5.0))  == doctest::Approx(-5.0).epsilon(0.001));
        CHECK((double)(-Distance(-3.0)) == doctest::Approx(3.0).epsilon(0.001));
        CHECK((double)(-Distance(0.0))  == doctest::Approx(0.0).epsilon(0.001));
    }

    TEST_CASE("cross-precision arithmetic widens to higher precision")
    {
        TFixed<8>  a(3);
        TFixed<12> b(2);
        auto sum  = a + b;
        auto prod = a * b;
        CHECK((double)sum  == doctest::Approx(5.0).epsilon(0.001));
        CHECK((double)prod == doctest::Approx(6.0).epsilon(0.01));
    }

    // -------------------------------------------------------------------------
    // Comparison
    // -------------------------------------------------------------------------

    TEST_CASE("equality and inequality")
    {
        Distance a(3.0), b(3.0), c(4.0);
        CHECK(a == b);
        CHECK(a != c);
    }

    TEST_CASE("ordering operators")
    {
        Distance lo(1.0), hi(5.0);
        CHECK(lo < hi);
        CHECK(hi > lo);
        CHECK(lo <= lo);
        CHECK(hi >= hi);
        CHECK(!(hi < lo));
    }

    TEST_CASE("comparison with double")
    {
        Distance a(7.0);
        CHECK(a == 7.0);
        CHECK(a != 8.0);
        CHECK(a <  8.0);
        CHECK(a >  6.0);
        CHECK(a <= 7.0);
        CHECK(a >= 7.0);
    }

    // -------------------------------------------------------------------------
    // Math utilities
    // -------------------------------------------------------------------------

    TEST_CASE("Abs")
    {
        CHECK(Abs(Distance(-3.0)) == Distance(3.0));
        CHECK(Abs(Distance( 4.0)) == Distance(4.0));
        CHECK(Abs(Distance( 0.0)) == Distance(0.0));
    }

    TEST_CASE("Min and Max")
    {
        Distance a(2.0), b(7.0);
        CHECK(Min(a, b) == a);
        CHECK(Max(a, b) == b);
        CHECK(Min(b, a) == a);
        CHECK(Max(a, a) == a);
    }

    TEST_CASE("Floor rounds down for positive non-integers")
    {
        CHECK(Floor(Distance(1.75)) == Distance(1));
        CHECK(Floor(Distance(0.99)) == Distance(0));
        CHECK(Floor(Distance(5.0))  == Distance(5));
    }

    TEST_CASE("Ceil rounds up for non-integer values")
    {
        CHECK(Ceil(Distance(1.25)) == Distance(2));
        CHECK(Ceil(Distance(0.01)) == Distance(1));
        CHECK(Ceil(Distance(3.75)) == Distance(4));
    }

    TEST_CASE("Round rounds to nearest")
    {
        CHECK(Round(Distance(1.25)) == Distance(1));
        CHECK(Round(Distance(1.75)) == Distance(2));
        CHECK(Round(Distance(2.5))  == Distance(3));
        CHECK(Round(Distance(0.49)) == Distance(0));
    }

    TEST_CASE("Clamp restricts to [lo, hi]")
    {
        Distance lo(0.0), hi(10.0);
        CHECK(Clamp(Distance(5.0),   lo, hi) == Distance(5.0));
        CHECK(Clamp(Distance(-1.0),  lo, hi) == Distance(0.0));
        CHECK(Clamp(Distance(11.0),  lo, hi) == Distance(10.0));
        CHECK(Clamp(Distance(0.0),   lo, hi) == Distance(0.0));
        CHECK(Clamp(Distance(10.0),  lo, hi) == Distance(10.0));
    }

    TEST_CASE("Clamp01 restricts to [0, 1]")
    {
        CHECK(Clamp01(Distance(0.5))  == Distance(0.5));
        CHECK(Clamp01(Distance(-0.5)) == Distance(0.0));
        CHECK(Clamp01(Distance(1.5))  == Distance(1.0));
    }

    TEST_CASE("Lerp interpolates linearly")
    {
        Distance a(0.0), b(10.0);
        CHECK((double)Lerp(a, b, Distance(0.0))  == doctest::Approx(0.0).epsilon(0.01));
        CHECK((double)Lerp(a, b, Distance(1.0))  == doctest::Approx(10.0).epsilon(0.01));
        CHECK((double)Lerp(a, b, Distance(0.5))  == doctest::Approx(5.0).epsilon(0.1));
        CHECK((double)Lerp(a, b, Distance(0.25)) == doctest::Approx(2.5).epsilon(0.1));
    }

    TEST_CASE("Lerp01 clamps t outside [0,1]")
    {
        Distance a(0.0), b(10.0);
        CHECK((double)Lerp01(a, b, Distance(2.0))  == doctest::Approx(10.0).epsilon(0.01));
        CHECK((double)Lerp01(a, b, Distance(-1.0)) == doctest::Approx(0.0).epsilon(0.01));
    }
}
