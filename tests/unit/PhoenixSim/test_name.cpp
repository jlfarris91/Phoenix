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
}
