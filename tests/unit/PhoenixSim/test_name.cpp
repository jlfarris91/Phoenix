#include <doctest/doctest.h>
#include <PhoenixSim/Name.h>

using namespace Phoenix;

TEST_SUITE("FName")
{
    TEST_CASE("same string produces same hash")
    {
        CHECK(FName("hello") == FName("hello"));
    }

    TEST_CASE("different strings produce different hashes")
    {
        CHECK(FName("hello") != FName("world"));
    }

    TEST_CASE("default-constructed names are equal")
    {
        CHECK(FName() == FName());
    }

    TEST_CASE("Append builds a compound name")
    {
        FName base("Phoenix");
        FName compound = base.Append("Unit");
        CHECK(compound != base);
        CHECK(compound == FName("Phoenix").Append("Unit"));
    }

    // -------------------------------------------------------------------------
    // Sentinel values
    // -------------------------------------------------------------------------

    TEST_CASE("None equals default-constructed FName")
    {
        CHECK(FName::None == FName());
    }

    TEST_CASE("Empty equals empty-string FName")
    {
        CHECK(FName::Empty == FName(""));
    }

    TEST_CASE("None and Empty are not equal to each other")
    {
        CHECK(FName::None != FName::Empty);
    }

    TEST_CASE("IsNoneOrEmpty recognises None")
    {
        CHECK(FName::IsNoneOrEmpty(FName::None));
    }

    TEST_CASE("IsNoneOrEmpty recognises Empty")
    {
        CHECK(FName::IsNoneOrEmpty(FName::Empty));
    }

    TEST_CASE("IsNoneOrEmpty rejects a non-empty name")
    {
        CHECK_FALSE(FName::IsNoneOrEmpty(FName("hello")));
    }

    // -------------------------------------------------------------------------
    // User-defined literal
    // -------------------------------------------------------------------------

    TEST_CASE("string literal operator _n produces same hash as constructor")
    {
        CHECK("hello"_n == FName("hello"));
    }

    // -------------------------------------------------------------------------
    // Ordering
    // -------------------------------------------------------------------------

    TEST_CASE("operator<=> provides total order consistent with equality")
    {
        FName a("alpha");
        FName b("beta");

        // Either a < b or b < a, but not both, and not equal.
        bool lt = (a <=> b) < 0;
        bool gt = (a <=> b) > 0;
        CHECK(lt != gt);
        CHECK((a <=> a) == 0);
    }

    // -------------------------------------------------------------------------
    // Implicit conversion to hash32_t
    // -------------------------------------------------------------------------

    TEST_CASE("implicit conversion to hash32_t matches underlying value")
    {
        FName name("conversion_test");
        hash32_t h = static_cast<hash32_t>(name);
        CHECK(FName(h) == name);
    }

    // -------------------------------------------------------------------------
    // std::hash specialisation
    // -------------------------------------------------------------------------

    TEST_CASE("std::hash produces consistent results")
    {
        FName name("std_hash_test");
        std::hash<FName> hasher;
        CHECK(hasher(name) == hasher(name));
        CHECK(hasher(FName("a")) != hasher(FName("b")));
    }

    // -------------------------------------------------------------------------
    // Append semantics
    // -------------------------------------------------------------------------

    TEST_CASE("Append with zero-value base behaves like constructing from appended string")
    {
        // When base Value == 0 (None), Append("foo") is the same as FName("foo").
        FName result = FName::None.Append("foo");
        CHECK(result == FName("foo"));
    }

    TEST_CASE("Append is not commutative")
    {
        CHECK(FName("a").Append("b") != FName("b").Append("a"));
    }

    TEST_CASE("operator+ is equivalent to Append")
    {
        FName base("base");
        CHECK((base + "suffix") == base.Append("suffix"));
    }

    // -------------------------------------------------------------------------
    // Combine semantics
    // -------------------------------------------------------------------------

    TEST_CASE("Combine with None returns other name unchanged")
    {
        FName name("combine_test");
        CHECK(FName::None.Combine(name) == name);
    }

    TEST_CASE("Combine produces a different result than Append")
    {
        FName a("foo");
        FName b("bar");
        // Combine hashes a name-of-a-name; Append hashes the raw bytes of "bar".
        // They must differ for non-trivial inputs.
        CHECK(a.Combine(b) != a.Append("bar"));
    }
}
