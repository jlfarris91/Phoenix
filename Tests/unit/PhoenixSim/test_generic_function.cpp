#include <doctest/doctest.h>
#include <PhoenixSim/Reflection/GenericFunction.h>
#include <PhoenixSim/Reflection/GenericValue.h>

using namespace Phoenix;

namespace
{
    static int Add(int a, int b) { return a + b; }
    static void Noop() {}

    struct Counter
    {
        int Value = 0;
        void Increment() { ++Value; }
        int Get() const { return Value; }
        int AddTo(int x) const { return Value + x; }
    };
}

TEST_SUITE("GenericFunction")
{
    TEST_CASE("free function: HasSelfParam is false")
    {
        auto fn = MakeGenericFunction(Add);
        CHECK(fn.HasSelfParam == false);
    }

    TEST_CASE("free function: invokes correctly")
    {
        auto fn = MakeGenericFunction(Add);
        GenericValue args[2] = { GenericValue::Borrow((int32_t)3), GenericValue::Borrow((int32_t)4) };
        GenericValue result = fn.Invoke(nullptr, args);
        CHECK(result.As<int32_t>() == 7);
    }

    TEST_CASE("void free function: returns Void")
    {
        auto fn = MakeGenericFunction(Noop);
        GenericValue result = fn.Invoke(nullptr, {});
        CHECK(result.Type.IsVoid());
    }

    TEST_CASE("instance method: HasSelfParam is true")
    {
        auto fn = MakeGenericFunction(&Counter::Increment);
        CHECK(fn.HasSelfParam == true);
    }

    TEST_CASE("instance method: mutates object")
    {
        auto fn = MakeGenericFunction(&Counter::Increment);
        Counter c;
        fn.Invoke(&c, {});
        CHECK(c.Value == 1);
    }

    TEST_CASE("const instance method: returns value")
    {
        auto fn = MakeGenericFunction(&Counter::AddTo);
        Counter c; c.Value = 10;
        GenericValue args[1] = { GenericValue::Borrow((int32_t)5) };
        GenericValue result = fn.Invoke(&c, args);
        CHECK(result.As<int32_t>() == 15);
    }
}
