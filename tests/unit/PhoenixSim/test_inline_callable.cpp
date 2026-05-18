
#include <doctest/doctest.h>

#include <atomic>
#include <memory>
#include <utility>

#include "PhoenixSim/InlineCallable.h"

using namespace Phoenix;

TEST_SUITE("TInlineCallable")
{
    TEST_CASE("default-constructed is empty")
    {
        TInlineCallable<void(), 64> c;
        CHECK_FALSE(static_cast<bool>(c));
    }

    TEST_CASE("invokes a stateless lambda")
    {
        int counter = 0;
        TInlineCallable<void(), 64> c = [&counter] { ++counter; };
        CHECK(static_cast<bool>(c));
        c();
        CHECK(counter == 1);
        c();
        CHECK(counter == 2);
    }

    TEST_CASE("supports return values and arguments")
    {
        TInlineCallable<int(int, int), 64> add = [](int a, int b) { return a + b; };
        CHECK(add(2, 3) == 5);
    }

    TEST_CASE("captures by value are copied into the inline storage")
    {
        int captured = 42;
        TInlineCallable<int(), 64> c = [captured] { return captured; };
        captured = 99; // mutating the original must not affect the capture
        CHECK(c() == 42);
    }

    TEST_CASE("captures by reference observe outer mutations")
    {
        int x = 0;
        TInlineCallable<void(), 64> c = [&x] { x += 10; };
        c();
        CHECK(x == 10);
        c();
        CHECK(x == 20);
    }

    TEST_CASE("copy semantics: each copy holds an independent body")
    {
        struct Counter
        {
            int n = 0;
            int operator()() { return ++n; }
        };
        TInlineCallable<int(), 64> a = Counter{};
        CHECK(a() == 1);
        CHECK(a() == 2);

        TInlineCallable<int(), 64> b = a; // copy
        CHECK(a() == 3);
        CHECK(b() == 3); // copy started from a's state (n=2), then incremented
    }

    TEST_CASE("move semantics: source is left empty")
    {
        int x = 0;
        TInlineCallable<void(), 64> src = [&x] { ++x; };
        CHECK(static_cast<bool>(src));

        TInlineCallable<void(), 64> dst = std::move(src);
        CHECK(static_cast<bool>(dst));
        CHECK_FALSE(static_cast<bool>(src));

        dst();
        CHECK(x == 1);
    }

    TEST_CASE("destructor runs the captured object's destructor exactly once")
    {
        struct Tracker
        {
            std::shared_ptr<int> Ref;
            Tracker(std::shared_ptr<int> r) : Ref(std::move(r)) {}
            void operator()() const {}
        };

        auto ref = std::make_shared<int>(0);
        CHECK(ref.use_count() == 1);
        {
            TInlineCallable<void(), 64> c = Tracker{ref};
            CHECK(ref.use_count() == 2); // captured a copy
        }
        CHECK(ref.use_count() == 1); // destructor reclaimed the captured copy
    }

    TEST_CASE("move-construction transfers ownership without leaking")
    {
        auto ref = std::make_shared<int>(0);
        {
            TInlineCallable<void(), 64> src = [ref] { (void)ref; };
            CHECK(ref.use_count() == 2);
            TInlineCallable<void(), 64> dst = std::move(src);
            CHECK(ref.use_count() == 2); // moved, not duplicated
        }
        CHECK(ref.use_count() == 1);
    }

    TEST_CASE("copy-assignment overwrites previous body and runs its destructor")
    {
        auto refA = std::make_shared<int>(0);
        auto refB = std::make_shared<int>(0);
        TInlineCallable<void(), 64> c = [refA] { (void)refA; };
        CHECK(refA.use_count() == 2);
        c = TInlineCallable<void(), 64>([refB] { (void)refB; });
        CHECK(refA.use_count() == 1); // old body destroyed
        CHECK(refB.use_count() == 2);
    }

    TEST_CASE("accepts a std::function as the source (via the templated constructor)")
    {
        std::function<int()> fn = [] { return 7; };
        TInlineCallable<int(), 128> c = std::move(fn);
        CHECK(c() == 7);
    }
}
