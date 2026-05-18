
#include <doctest/doctest.h>

#include "Phoenix.Sim/Containers/Optional.h"

using namespace Phoenix;

TEST_SUITE("TOptional")
{
    TEST_CASE("default-constructed is not set")
    {
        TOptional<int> opt;
        CHECK(!opt.IsSet());
    }

    TEST_CASE("value-constructed is set with correct value")
    {
        TOptional<int> opt(42);
        CHECK(opt.IsSet());
        CHECK(opt.Get() == 42);
    }

    TEST_CASE("operator* dereferences value")
    {
        TOptional<int> opt(7);
        CHECK(*opt == 7);
    }

    TEST_CASE("operator-> accesses members")
    {
        struct Point { int X = 0, Y = 0; };
        TOptional<Point> opt(Point{3, 4});
        CHECK(opt->X == 3);
        CHECK(opt->Y == 4);
    }

    TEST_CASE("assign from T marks as set")
    {
        TOptional<int> opt;
        opt = 99;
        CHECK(opt.IsSet());
        CHECK(opt.Get() == 99);
    }

    TEST_CASE("assign from T overwrites previous value")
    {
        TOptional<int> opt(1);
        opt = 2;
        CHECK(opt.Get() == 2);
    }

    TEST_CASE("Reset marks as not set")
    {
        TOptional<int> opt(5);
        opt.Reset();
        CHECK(!opt.IsSet());
    }

    TEST_CASE("GetValue returns stored value when set")
    {
        TOptional<int> opt(10);
        CHECK(opt.GetValue(42) == 10);
    }

    TEST_CASE("GetValue returns default when not set")
    {
        TOptional<int> opt;
        CHECK(opt.GetValue(42) == 42);
    }

    TEST_CASE("equality with T")
    {
        TOptional<int> opt(5);
        CHECK(opt == 5);
        CHECK(opt != 6);
    }

    TEST_CASE("equality with another TOptional — same value")
    {
        TOptional<int> a(5), b(5);
        CHECK(a == b);
    }

    TEST_CASE("equality with another TOptional — different values")
    {
        TOptional<int> a(5), b(6);
        CHECK(a != b);
    }

    TEST_CASE("set vs unset TOptional are not equal")
    {
        TOptional<int> set(5), unset;
        CHECK(set != unset);
    }

    TEST_CASE("copy construction preserves value and state")
    {
        TOptional<int> a(33);
        TOptional<int> b(a);
        CHECK(b.IsSet());
        CHECK(b.Get() == 33);
    }

    TEST_CASE("copy construction of unset preserves unset state")
    {
        TOptional<int> a;
        TOptional<int> b(a);
        CHECK(!b.IsSet());
    }

    TEST_CASE("copy assignment from set optional")
    {
        TOptional<int> a(77), b;
        b = a;
        CHECK(b.IsSet());
        CHECK(b.Get() == 77);
    }

    TEST_CASE("move construction transfers value")
    {
        TOptional<int> a(55);
        TOptional<int> b(std::move(a));
        CHECK(b.IsSet());
        CHECK(b.Get() == 55);
    }
}
