#include <doctest/doctest.h>
#include <Phoenix.Sim/Reflection/GenericFunction.h>
#include <Phoenix.Sim/Reflection/Variant.h>

#include "Phoenix.Sim/Reflection/TypeDescriptor.h"

using namespace Phoenix;

namespace
{
    int Add(int a, int b) { return a + b; }
    void Noop() {}

    struct Counter
    {
        int Value = 0;
        void Increment() { ++Value; }
        int AddTo(int x) const { return Value + x; }
    };
}

TEST_SUITE("GenericFunction")
{
    TEST_CASE("free function: HasSelfParam is false")
    {
        auto fn = MakeGenericFunction(Add);
        CHECK(fn.HasSelf() == false);
    }

    TEST_CASE("free function: invokes correctly")
    {
        auto fn = MakeGenericFunction(Add);
        Variant args[2] = { Variant(3), Variant(4) };
        Variant result = fn(nullptr, args);
        CHECK(result.As<int32_t>() == 7);
    }

    TEST_CASE("void free function: returns Void")
    {
        auto fn = MakeGenericFunction(Noop);
        Variant result = fn(nullptr, {});
        CHECK(result == Variant::Void());
    }

    TEST_CASE("instance method: HasSelfParam is true")
    {
        auto fn = MakeGenericFunction(&Counter::Increment);
        CHECK(fn.HasSelf() == true);
    }

    TEST_CASE("instance method: mutates object")
    {
        auto fn = MakeGenericFunction(&Counter::Increment);
        Counter c;
        fn(&c, {});
        CHECK(c.Value == 1);
    }

    TEST_CASE("const instance method: returns value")
    {
        auto fn = MakeGenericFunction(&Counter::AddTo);
        Counter c; c.Value = 10;
        Variant args[1] = { Variant(5) };
        Variant result = fn(&c, args);
        CHECK(result.As<int32_t>() == 15);
    }
}
